#ifndef ASYNC_H
#define ASYNC_H

#include <uv.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "inference.h"
#include "cache.h"
#include "../../3libs/log.h"
#include "../../3libs/yyjson.h"
#include "../../3libs/sds.h"

// Globals for Event Loop
static uv_loop_t *loop;
static CURLM *curl_handle;
static uv_timer_t timeout;
static uv_timer_t fetch_timer;

typedef struct {
    char coin[16];
    char url[256];
    sds response;
    CURL *easy;
} FetchContext;

typedef struct curl_context_s {
    uv_poll_t poll_handle;
    curl_socket_t sockfd;
} curl_context_t;

typedef struct {
    uv_work_t req;
    char coin[16];
    Kline klines[SEQ_LEN + 2];
    int kline_count;
} InferenceTask;

// Forward declarations
static void check_multi_info(void);
static void curl_perform(uv_poll_t *req, int status, int events);
static void on_timeout(uv_timer_t *req);
static int start_timeout(CURLM *multi, long timeout_ms, void *userp);
static int handle_socket(CURL *easy, curl_socket_t s, int action, void *userp, void *socketp);

// --- Thread Pool Task: Run Model Inference ---
static void run_inference_task(uv_work_t *req) {
    InferenceTask *task = (InferenceTask*)req->data;
    const char *cname = task->coin;
    
    CoinCache *cache = get_coin_cache(cname);
    pthread_mutex_lock(&cache->coin_mutex);
    
    // Check if model needs loading (download synchronously inside worker thread)
    if (!cache->model) {
        uint8_t *l_model_buf = malloc(49152);
        if (l_model_buf) {
            int m_len = download_model_from_github(cname, l_model_buf, 49152);
            if (m_len > 0) {
                uint8_t pool_idx = (uint8_t)(cache - caches);
                cache->model = load_model(pool_idx, l_model_buf, m_len);
            }
            free(l_model_buf);
        }
    }

    if (!cache->model || task->kline_count < SEQ_LEN + 2) {
        cache->last_res.success = 0;
        stbsp_snprintf(cache->last_res.error_msg, sizeof(cache->last_res.error_msg), 
            !cache->model ? "Model error" : "Data error");
        pthread_mutex_unlock(&cache->coin_mutex);
        return;
    }

    // Prepare inputs
    cache->kline_count = task->kline_count;
    memcpy(cache->klines, task->klines, task->kline_count * sizeof(Kline));
    for (int i = 0; i < SEQ_LEN; i++) {
        float c1 = (float)cache->klines[i].close, c2 = (float)cache->klines[i+1].close;
        cache->input[i] = (c1 > 1e-9f && c2 > 1e-9f) ? logf(c2 / c1) : 0;
    }
    update_cache_day(cache);

    struct timeval start, end; gettimeofday(&start, NULL);
    float p_val = predict(cache->model, cache->input);
    gettimeofday(&end, NULL);

    cache->pred_log_diff = p_val;
    cache->last_res.success = !isnan(p_val) && !isinf(p_val);
    cache->last_res.inference_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    cache->last_res.pred_change_pct = cache->last_res.success ? (expf(p_val) - 1.0f) * 100.0f : 0;
    
    time_t pred_ts = (cache->klines[cache->kline_count-1].timestamp) / 1000 + 86400;
    struct tm t_pred; gmtime_r(&pred_ts, &t_pred);
    strftime(cache->last_res.date, sizeof(cache->last_res.date), "%Y-%m-%d", &t_pred);

    cache->last_res.last_price = (float)cache->klines[task->kline_count-1].close;
    float yesterday_close = (float)cache->klines[task->kline_count-2].close;
    cache->last_res.change_pct = (yesterday_close > 1e-9) ? ((cache->last_res.last_price - yesterday_close) / yesterday_close) * 100.0f : 0;
    cache->last_res.pred_price = cache->last_res.last_price * expf(cache->pred_log_diff);
    cache->last_res.trend = (cache->last_res.pred_price > cache->last_res.last_price) ? 1 : 0;
    
    pthread_mutex_unlock(&cache->coin_mutex);
}

static void on_inference_done(uv_work_t *req, int status) {
    InferenceTask *task = (InferenceTask*)req->data;
    free(task);
}

// --- Network Event Loop Callbacks ---
static curl_context_t* create_curl_context(curl_socket_t sockfd) {
    curl_context_t *context = (curl_context_t *)malloc(sizeof(*context));
    context->sockfd = sockfd;
    uv_poll_init_socket(loop, &context->poll_handle, sockfd);
    context->poll_handle.data = context;
    return context;
}

static void curl_close_cb(uv_handle_t *handle) {
    curl_context_t *context = (curl_context_t *)handle->data;
    free(context);
}

static void destroy_curl_context(curl_context_t *context) {
    uv_close((uv_handle_t *)&context->poll_handle, curl_close_cb);
}

static int handle_socket(CURL *easy, curl_socket_t s, int action, void *userp, void *socketp) {
    curl_context_t *curl_context = (curl_context_t *)socketp;
    int events = 0;

    switch(action) {
    case CURL_POLL_IN:
    case CURL_POLL_OUT:
    case CURL_POLL_INOUT:
        if (!curl_context) {
            curl_context = create_curl_context(s);
            curl_multi_assign(curl_handle, s, (void *)curl_context);
        }
        if (action != CURL_POLL_IN) events |= UV_WRITABLE;
        if (action != CURL_POLL_OUT) events |= UV_READABLE;
        uv_poll_start(&curl_context->poll_handle, events, curl_perform);
        break;
    case CURL_POLL_REMOVE:
        if (curl_context) {
            uv_poll_stop(&curl_context->poll_handle);
            destroy_curl_context(curl_context);
            curl_multi_assign(curl_handle, s, NULL);
        }
        break;
    }
    return 0;
}

static int start_timeout(CURLM *multi, long timeout_ms, void *userp) {
    if (timeout_ms < 0) {
        uv_timer_stop(&timeout);
    } else {
        if (timeout_ms == 0) timeout_ms = 1;
        uv_timer_start(&timeout, on_timeout, timeout_ms, 0);
    }
    return 0;
}

static void curl_perform(uv_poll_t *req, int status, int events) {
    int running_handles;
    int flags = 0;
    if (events & UV_READABLE) flags |= CURL_CSELECT_IN;
    if (events & UV_WRITABLE) flags |= CURL_CSELECT_OUT;
    
    curl_context_t *context = (curl_context_t *)req->data;
    curl_multi_socket_action(curl_handle, context->sockfd, flags, &running_handles);
    check_multi_info();
}

static void on_timeout(uv_timer_t *req) {
    int running_handles;
    curl_multi_socket_action(curl_handle, CURL_SOCKET_TIMEOUT, 0, &running_handles);
    check_multi_info();
}

static void check_multi_info(void) {
    CURLMsg *message;
    int pending;
    
    while ((message = curl_multi_info_read(curl_handle, &pending))) {
        switch (message->msg) {
        case CURLMSG_DONE: {
            CURL *easy_handle = message->easy_handle;
            FetchContext *ctx;
            curl_easy_getinfo(easy_handle, CURLINFO_PRIVATE, &ctx);
            
            long http_code = 0;
            curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &http_code);
            
            if (message->data.result == CURLE_OK && http_code == 200 && sdslen(ctx->response) > 0) {
                yyjson_doc *doc = yyjson_read(ctx->response, sdslen(ctx->response), 0);
                if (doc) {
                    InferenceTask *task = malloc(sizeof(InferenceTask));
                    memset(task, 0, sizeof(InferenceTask));
                    snprintf(task->coin, sizeof(task->coin), "%s", ctx->coin);
                    
                    yyjson_val *root = yyjson_doc_get_root(doc);
                    int n = 0;
                    if (yyjson_is_arr(root)) {
                        size_t idx, max; yyjson_val *item;
                        yyjson_arr_foreach(root, idx, max, item) {
                            if (n >= SEQ_LEN + 2) break;
                            if (yyjson_is_arr(item)) {
                                yyjson_val *ts = yyjson_arr_get(item, 0);
                                yyjson_val *cl = yyjson_arr_get(item, 4);
                                if (ts && cl && yyjson_is_str(cl)) {
                                    task->klines[n].timestamp = (long long)yyjson_get_num(ts);
                                    task->klines[n].close = atof(yyjson_get_str(cl));
                                    if (task->klines[n].close > 0) n++;
                                }
                            }
                        }
                    }
                    yyjson_doc_free(doc);
                    
                    if (n >= SEQ_LEN + 2) {
                        task->kline_count = n;
                        task->req.data = task;
                        // Submit to Thread Pool for CPU heavy inference
                        uv_queue_work(loop, &task->req, run_inference_task, on_inference_done);
                    } else {
                        free(task);
                    }
                }
            } else {
                log_error("Fetch failed for %s", ctx->coin);
            }
            
            curl_multi_remove_handle(curl_handle, easy_handle);
            curl_easy_cleanup(easy_handle);
            sdsfree(ctx->response);
            free(ctx);
            break;
        }
        default:
            break;
        }
    }
}

static size_t async_write_cb(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    sds *str = (sds *)userp;
    *str = sdscatlen(*str, contents, realsize);
    return realsize;
}

static void add_download(const char *coin) {
    FetchContext *ctx = malloc(sizeof(FetchContext));
    strncpy(ctx->coin, coin, sizeof(ctx->coin)-1);
    ctx->coin[sizeof(ctx->coin)-1] = 0;
    stbsp_snprintf(ctx->url, sizeof(ctx->url), "https://api.binance.com/api/v3/klines?symbol=%sUSDT&interval=1d&limit=%d", coin, SEQ_LEN + 2);
    ctx->response = sdsempty();
    
    CURL *easy = curl_easy_init();
    ctx->easy = easy;
    
    curl_easy_setopt(easy, CURLOPT_URL, ctx->url);
    curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, async_write_cb);
    curl_easy_setopt(easy, CURLOPT_WRITEDATA, &ctx->response);
    curl_easy_setopt(easy, CURLOPT_PRIVATE, ctx);
    curl_easy_setopt(easy, CURLOPT_USERAGENT, "A35M-Engine/1.0");
    curl_easy_setopt(easy, CURLOPT_TIMEOUT, 15L);
    
    curl_multi_add_handle(curl_handle, easy);
}

// Track these externally
extern const char *g_coins[];
extern int g_num_coins;

static void trigger_fetch_all(uv_timer_t* handle) {
    for (int i = 0; i < g_num_coins; i++) {
        add_download(g_coins[i]);
    }
}

static inline void* start_async_engine(void *arg) {
    // Set 4 threads for libuv thread pool
    setenv("UV_THREADPOOL_SIZE", "4", 1);
    
    loop = uv_default_loop();
    
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_multi_init();
    curl_multi_setopt(curl_handle, CURLMOPT_SOCKETFUNCTION, handle_socket);
    curl_multi_setopt(curl_handle, CURLMOPT_TIMERFUNCTION, start_timeout);
    
    uv_timer_init(loop, &timeout);
    uv_timer_init(loop, &fetch_timer);
    
    // Initial fetch immediately
    trigger_fetch_all(&fetch_timer);
    
    // Then every 60 seconds (60000 ms)
    uv_timer_start(&fetch_timer, trigger_fetch_all, 60000, 60000);
    
    log_info("Async engine starting (Event Loop + 4-Thread Pool)");
    uv_run(loop, UV_RUN_DEFAULT);
    
    curl_multi_cleanup(curl_handle);
    curl_global_cleanup();
    return NULL;
}

#endif // ASYNC_H

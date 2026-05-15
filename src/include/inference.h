#ifndef INFERENCE_H
#define INFERENCE_H

#include "models.h"
#include "cache.h"
#include "openSSL.h"
#include "cJSON.h"
#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>

#define BINANCE_HOST "api.binance.com"

// Per-coin mutex is now located inside CoinCache struct

static inline int fetch_binance_data(const char *symbol, Kline *out, int limit) {
    SSLConnection c = create_ssl_connection(BINANCE_HOST);
    if (!c.ssl) return 0;
    char req[256], *buf = malloc(131072);
    if (!buf) { cleanup_ssl_connection(c); return 0; }
    int n = 0, len, pos = 0;
    snprintf(req, 256, "GET /api/v3/klines?symbol=%s&interval=1d&limit=%d HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", symbol, limit, BINANCE_HOST);
    if (SSL_write(c.ssl, req, (int)strlen(req)) <= 0) { free(buf); cleanup_ssl_connection(c); return 0; }
    while (pos < 131071 && (len = SSL_read(c.ssl, buf + pos, 131071 - pos)) > 0) pos += len;
    buf[pos] = 0;
    cleanup_ssl_connection(c);
    if (pos >= 131071) { free(buf); return 0; }
    char *body = strstr(buf, "\r\n\r\n");
    if (!body) { free(buf); return 0; }
    body += 4;
    cJSON *root = cJSON_Parse(body);
    if (root) {
        cJSON *item;
        cJSON_ArrayForEach(item, root) {
            if (n >= limit) break;
            cJSON *ts = cJSON_GetArrayItem(item, 0);
            cJSON *cl = cJSON_GetArrayItem(item, 4);
            if (ts && cl && cl->valuestring && cl->valuestring[0]) {
                out[n].timestamp = (long long)ts->valuedouble;
                out[n].close = atof(cl->valuestring);
                if (out[n].close > 0) n++;
            }
        }
        cJSON_Delete(root);
    }
    free(buf);
    return n;
}

static inline int download_model_from_github(const char *coin, uint8_t *out, int sz) {
    SSLConnection c = create_ssl_connection("raw.githubusercontent.com");
    if (!c.ssl) return 0;
    char req[256], *b;
    int n = 0, r, h = 0;
    snprintf(req, 256, "GET /KhanhAI-VN/Test/main/%s.bin HTTP/1.0\r\nHost: raw.githubusercontent.com\r\nConnection: close\r\n\r\n", coin);
    if (SSL_write(c.ssl, req, (int)strlen(req)) <= 0) { cleanup_ssl_connection(c); return 0; }
    while (n < sz - 1 && (r = SSL_read(c.ssl, out + n, sz - 1 - n)) > 0) {
        n += r; out[n] = 0;
        if (!h && (b = strstr((char*)out, "\r\n\r\n"))) {
            char *status = strstr((char*)out, " ");
            if (!status || atoi(status + 1) != 200) { h = -1; break; }
            char *cl_hdr = strcasestr((char*)out, "Content-Length:");
            h = (int)((b + 4) - (char*)out);
            if (cl_hdr && cl_hdr < b) {
                char *cl_val = strchr(cl_hdr, ':');
                if (cl_val) {
                    cl_val++;
                    while (*cl_val == ' ') cl_val++;
                    if (*cl_val >= '0' && *cl_val <= '9') {
                        if (atoi(cl_val) > (sz - h)) { h = -1; break; }
                    }
                }
            }
        }
    }
    cleanup_ssl_connection(c);
    if (h <= 0 || h >= n) return 0;
    memmove(out, out + h, n - h);
    return n - h;
}

static inline PredictionResult run_prediction(const char *coin) {
    const char *cname = coin ? coin : "BTC";
    for (const char *p = cname; *p; p++) {
        if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9'))) {
            return (PredictionResult){.success = 0, .coin = "", .error_msg = "Invalid symbol"};
        }
    }
    char symbol[32]; snprintf(symbol, sizeof(symbol), "%sUSDT", cname);
    
    CoinCache *cache = get_coin_cache(cname);
    pthread_mutex_lock(&cache->coin_mutex);
    
    if (should_update_cache(cache)) {
        pthread_mutex_unlock(&cache->coin_mutex);
        
        // Allocate local buffers for downloading data to avoid shared-state bottlenecks
        uint8_t *l_model_buf = malloc(49152);
        Kline *l_kline_buf = malloc((SEQ_LEN + 2) * sizeof(Kline));
        if (!l_model_buf || !l_kline_buf) {
            free(l_model_buf); free(l_kline_buf);
            return (PredictionResult){.success = 0, .coin = "", .error_msg = "Memory error"};
        }

        int m_len = download_model_from_github(cname, l_model_buf, 49152);
        int f_count = fetch_binance_data(symbol, l_kline_buf, SEQ_LEN + 2);
        
        cache = get_coin_cache(cname);
        pthread_mutex_lock(&cache->coin_mutex);
        if (should_update_cache(cache)) {
            uint8_t pool_idx = (uint8_t)(cache - caches);
            if (m_len > 0) cache->model = load_model(pool_idx, l_model_buf, m_len);
            
            if (!cache->model || f_count < SEQ_LEN + 2) {
                PredictionResult res = {0};
                memcpy(res.coin, cache->coin, sizeof(res.coin));
                snprintf(res.error_msg, sizeof(res.error_msg), "%s", !cache->model ? "Model error" : "Data error");
                pthread_mutex_unlock(&cache->coin_mutex);
                free(l_model_buf); free(l_kline_buf);
                return res;
            }

            cache->kline_count = f_count;
            memcpy(cache->klines, l_kline_buf, f_count * sizeof(Kline));
            for (int i = 0; i < SEQ_LEN; i++) {
                float c1 = (float)cache->klines[i].close, c2 = (float)cache->klines[i+1].close;
                cache->input[i] = (c1 > 1e-9f && c2 > 1e-9f) ? logf(c2 / c1) : 0;
            }
            
            update_cache_day(cache);

            float local_input[SEQ_LEN];
            memcpy(local_input, cache->input, sizeof(local_input));
            Model *m = cache->model;

            struct timeval start, end; gettimeofday(&start, NULL);
            float p_val = predict(m, local_input);
            gettimeofday(&end, NULL);

            cache->pred_log_diff = p_val;
            cache->last_res.success = !isnan(p_val) && !isinf(p_val);
            cache->last_res.inference_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
            cache->last_res.pred_change_pct = cache->last_res.success ? (expf(p_val) - 1.0f) * 100.0f : 0;
            
            time_t pred_ts = (cache->klines[cache->kline_count-1].timestamp) / 1000 + 86400;
            struct tm t_pred; gmtime_r(&pred_ts, &t_pred);
            strftime(cache->last_res.date, sizeof(cache->last_res.date), "%Y-%m-%d", &t_pred);
        }
        free(l_model_buf); free(l_kline_buf);
    }

    // Secondary fetch only if needed for current price update (not full sync)
    pthread_mutex_unlock(&cache->coin_mutex);
    Kline latest[2] = {0};
    int n = fetch_binance_data(symbol, latest, 2);
    pthread_mutex_lock(&cache->coin_mutex);
    
    if (n == 2) {
        cache->last_res.last_price = (float)latest[1].close;
        float yesterday_close = (float)latest[0].close;
        cache->last_res.change_pct = (yesterday_close > 1e-9) ? ((cache->last_res.last_price - yesterday_close) / yesterday_close) * 100.0f : 0;
        cache->last_res.pred_price = cache->last_res.last_price * expf(cache->pred_log_diff);
        cache->last_res.trend = (cache->last_res.pred_price > cache->last_res.last_price) ? 1 : 0;
    }

    PredictionResult res = cache->last_res;
    memcpy(res.coin, cache->coin, sizeof(res.coin));
    pthread_mutex_unlock(&cache->coin_mutex);
    return res;
}

static inline void trigger_github_retrain(const char *token) {
    SSLConnection c = create_ssl_connection("api.github.com");
    if (!c.ssl) return;
    char req[1024], res[1024];
    const char *body = "{\"ref\":\"main\"}";
    int req_len = snprintf(req, sizeof(req), 
        "POST /repos/KhanhAI-VN/Test/actions/workflows/retrain.yml/dispatches HTTP/1.1\r\n"
        "Host: api.github.com\r\n"
        "Accept: application/vnd.github+json\r\n"
        "Authorization: Bearer %s\r\n"
        "X-GitHub-Api-Version: 2022-11-28\r\n"
        "User-Agent: Luckfox-Pico\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n\r\n"
        "%s", token, strlen(body), body);
    if (req_len >= 0 && req_len < (int)sizeof(req)) {
        send_http_request(&c, req, res, sizeof(res));
    }
    cleanup_ssl_connection(c);
}

#endif

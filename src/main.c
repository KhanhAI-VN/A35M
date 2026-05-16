#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include "include/inference.h"
#include "include/cache.h"
#include "include/async.h"
#include "../3libs/log.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "../3libs/stb_sprintf.h"

static char html[8192], css[8192];
static uint8_t logo[32768];
static size_t logo_sz = 0;
static char retrain_token[128] = {0};
static int last_retrain_day = -1;
static int last_retrain_year = -1;

const char *g_coins[] = {"BTC", "ETH", "SOL", "SHIB", "ADA"};
int g_num_coins = 5;

static enum MHD_Result send_res(struct MHD_Connection *c, const char *body, int code, const char *type) {
    struct MHD_Response *r = MHD_create_response_from_buffer(strlen(body), (void*)body, MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(r, "Content-Type", type);
    MHD_add_response_header(r, "Access-Control-Allow-Origin", "*");
    enum MHD_Result ret = MHD_queue_response(c, code, r);
    MHD_destroy_response(r);
    return ret;
}

static void load(const char *p, char *b) {
    FILE *f = fopen(p, "r");
    if (f) { b[fread(b, 1, 8191, f)] = 0; fclose(f); }
}

static void load_bin(const char *p, uint8_t *b, size_t max_sz, size_t *sz) {
    FILE *f = fopen(p, "rb");
    if (f) { *sz = fread(b, 1, max_sz, f); fclose(f); }
}

static void load_env() {
    FILE *f = fopen(".env", "r");
    if (!f) return;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "Retrain =", 9) == 0) {
            char *val = line + 9;
            while (*val == ' ') val++;
            strncpy(retrain_token, val, sizeof(retrain_token) - 1);
            retrain_token[sizeof(retrain_token) - 1] = 0;
            char *end = retrain_token + strlen(retrain_token) - 1;
            while (end >= retrain_token && (*end == '\n' || *end == '\r' || *end == ' ')) *end-- = 0;
        }
    }
    fclose(f);
}

void* scheduler_thread(void* arg) {
    while (1) {
        time_t now = time(NULL);
        struct tm t; gmtime_r(&now, &t);
        if (t.tm_hour == 23 && (t.tm_yday != last_retrain_day || t.tm_year != last_retrain_year)) {
            log_info("Triggering scheduled retrain (23:00 UTC)...");
            trigger_github_retrain(retrain_token);
            last_retrain_day = t.tm_yday;
            last_retrain_year = t.tm_year;
        }
        sleep(60);
    }
    return NULL;
}

static enum MHD_Result handler(void *cls, struct MHD_Connection *c, const char *url, const char *meth, const char *v, const char *data, size_t *s, void **ptr) {
    if (!strcmp(url, "/api/predict")) {
        const char *coin = MHD_lookup_connection_value(c, MHD_GET_ARGUMENT_KIND, "coin");
        const char *cname = coin ? coin : "BTC";
        CoinCache *cache = get_coin_cache(cname);
        pthread_mutex_lock(&cache->coin_mutex);
        PredictionResult r = cache->last_res;
        pthread_mutex_unlock(&cache->coin_mutex);
        
        char json[256];
        stbsp_snprintf(json, 256, "{\"success\":%s,\"price\":%.2f,\"change\":%.2f,\"trend\":\"%s\"}", 
                 r.success ? "true" : "false", r.last_price, r.change_pct, r.trend ? "UP" : "DOWN");
        return send_res(c, json, 200, "application/json");
    }
    if (!strcmp(url, "/logo.png") || !strcmp(url, "/favicon.ico")) {
        struct MHD_Response *r = MHD_create_response_from_buffer(logo_sz, (void*)logo, MHD_RESPMEM_MUST_COPY);
        MHD_add_response_header(r, "Content-Type", "image/png");
        enum MHD_Result ret = MHD_queue_response(c, 200, r);
        MHD_destroy_response(r);
        return ret;
    }
    int is_css = !strcmp(url, "/web.css");
    char *buf = is_css ? css : html;
    return buf[0] ? send_res(c, buf, 200, is_css ? "text/css" : "text/html") : send_res(c, "404", 404, "text/plain");
}

static void* prediction_wrapper(void *arg) {
    run_prediction((const char*)arg);
    return NULL;
}

int main(int argc, char **argv) {
    const char *coins[] = {"BTC", "ETH", "SOL", "SHIB", "ADA"};
    load_env();
    if (argc > 1) {
        printf("\n  ASSET       PRICE           PRED     CHANGE\n  ───────────────────────────────────────────\n");
        if (!strcmp(argv[1], "ALL")) {
            pthread_t tids[5];
            for (int i = 0; i < 5; i++) {
                pthread_create(&tids[i], NULL, prediction_wrapper, (void*)coins[i]);
            }
            for (int i = 0; i < 5; i++) {
                pthread_join(tids[i], NULL);
                PredictionResult pr = run_prediction(coins[i]);
                if (pr.success) printf("  %-10s  $%-13.2f  %-7s (%+.2f%%)  %+.2f%%\n", coins[i], pr.last_price, pr.trend ? "UP" : "DOWN", pr.pred_change_pct, pr.change_pct);
            }
        } else {
            PredictionResult r = run_prediction(argv[1]);
            if (r.success) printf("  %-10s  $%-13.2f  %-7s (%+.2f%%)  %+.2f%%\n", argv[1], r.last_price, r.trend ? "UP" : "DOWN", r.pred_change_pct, r.change_pct);
        }
        printf("  ───────────────────────────────────────────\n\n");
        return 0;
    }

    if (retrain_token[0]) {
        pthread_t tid;
        pthread_create(&tid, NULL, scheduler_thread, NULL);
        pthread_detach(tid);
    }

    pthread_t async_tid;
    pthread_create(&async_tid, NULL, start_async_engine, NULL);
    pthread_detach(async_tid);

    load("src/web/web.html", html); 
    load("src/web/web.css", css);
    load_bin("src/web/logo.png", logo, sizeof(logo), &logo_sz);

    struct MHD_Daemon *d = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, 8080, 0, 0, &handler, 0, MHD_OPTION_END);
    if (!d) { log_fatal("Failed to start HTTP daemon on port 8080"); return 1; }
    log_info("Shrimp Dashboard started on http://localhost:8080");
    printf("Shrimp Dashboard: http://localhost:8080\nPress Enter to stop.\n");
    getchar(); MHD_stop_daemon(d); return 0;
}

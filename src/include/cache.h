#ifndef CACHE_H
#define CACHE_H

#include "models.h"
#include <time.h>
#include <string.h>

typedef struct {
    char coin[16];
    char date[32];
    float last_price;
    float pred_price;
    float change_pct;
    float pred_change_pct;
    int trend; // 1 for UP, 0 for DOWN
    double inference_ms;
    int success;
    char error_msg[128];
} PredictionResult;

typedef struct {
    char coin[16];
    Kline klines[SEQ_LEN + 2];
    int kline_count;
    float input[SEQ_LEN];
    float pred_log_diff;
    PredictionResult last_res;
    int last_sync_day;
    int last_sync_year;
    Model* model;
} CoinCache;

static CoinCache caches[MAX_CACHED_COINS];
static int num_cached_coins = 0;

static inline CoinCache* get_coin_cache(const char *coin) {
    int i, oi = 0, ot = 2e9;
    for (i = 0; i < num_cached_coins; i++) if (!strcmp(caches[i].coin, coin)) return &caches[i];
    if (num_cached_coins < MAX_CACHED_COINS) i = num_cached_coins++;
    else {
        for (int j = 0; j < MAX_CACHED_COINS; j++) {
            int t = caches[j].last_sync_year * 365 + caches[j].last_sync_day;
            if (t < ot) { ot = t; oi = j; }
        }
        i = oi;
    }
    CoinCache *c = &caches[i];
    memset(c, 0, sizeof(CoinCache));
    strncpy(c->coin, coin, 15);
    c->last_sync_day = -1;
    return c;
}

static inline int should_update_cache(CoinCache *c) {
    time_t now = time(0); struct tm t; gmtime_r(&now, &t);
    return (t.tm_yday != c->last_sync_day || t.tm_year != c->last_sync_year || !c->last_res.success);
}

static inline void update_cache_day(CoinCache *c) {
    time_t now = time(0); struct tm t; gmtime_r(&now, &t);
    c->last_sync_day = t.tm_yday; c->last_sync_year = t.tm_year;
}

#endif

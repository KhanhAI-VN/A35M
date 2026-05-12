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
    for (int i = 0; i < num_cached_coins; i++)
        if (strcmp(caches[i].coin, coin) == 0) return &caches[i];
    if (num_cached_coins < MAX_CACHED_COINS) {
        CoinCache *c = &caches[num_cached_coins++];
        memset(c, 0, sizeof(CoinCache));
        strcpy(c->coin, coin);
        c->last_sync_day = -1;
        return c;
    }
    return &caches[0]; // Fallback to first slot if full
}

static inline int should_update_cache(CoinCache *cache) {
    time_t now = time(NULL);
    struct tm *t = gmtime(&now);
    if (t->tm_yday != cache->last_sync_day || t->tm_year != cache->last_sync_year || !cache->last_res.success)
        return 1;
    return 0;
}

static inline void update_cache_day(CoinCache *cache) {
    time_t now = time(NULL);
    struct tm *t = gmtime(&now);
    cache->last_sync_day = t->tm_yday;
    cache->last_sync_year = t->tm_year;
}

#endif

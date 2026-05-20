#ifndef SAFE_CACHE_H
#define SAFE_CACHE_H

#include <pthread.h>
#include "models.h"

typedef struct {
  char coin[16];
  char date[32];
  float last_price;
  float pred_price;
  float change_pct;
  float pred_change_pct;
  float avg_volatility_pct;
  int trend;
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
  Model *model;
  pthread_mutex_t coin_mutex;
  int ref_count;
} CoinCache;

extern CoinCache caches[MAX_CACHED_COINS];

void safe_cache_init(void);
CoinCache *safe_cache_acquire(const char *coin);
void safe_cache_release(CoinCache *c);

#endif

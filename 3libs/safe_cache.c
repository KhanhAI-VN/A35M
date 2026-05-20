#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "safe_cache.h"

CoinCache caches[MAX_CACHED_COINS];
static int num_cached_coins = 0;
static pthread_mutex_t cache_internal_mutex = PTHREAD_MUTEX_INITIALIZER;

void safe_cache_init(void) {
  pthread_mutex_lock(&cache_internal_mutex);
  for (int i = 0; i < MAX_CACHED_COINS; i++) {
    memset(&caches[i], 0, sizeof(CoinCache));
    pthread_mutex_init(&caches[i].coin_mutex, NULL);
    caches[i].ref_count = 0;
  }
  num_cached_coins = 0;
  pthread_mutex_unlock(&cache_internal_mutex);
}

CoinCache *safe_cache_acquire(const char *coin) {
  pthread_mutex_lock(&cache_internal_mutex);

  // 1. Search for existing coin cache
  for (int i = 0; i < num_cached_coins; i++) {
    if (caches[i].coin[0] != '\0' && !strcmp(caches[i].coin, coin)) {
      caches[i].ref_count++;
      pthread_mutex_unlock(&cache_internal_mutex);
      return &caches[i];
    }
  }

  int target_idx = -1;
  // 2. If there is free space, allocate it
  if (num_cached_coins < MAX_CACHED_COINS) {
    target_idx = num_cached_coins++;
  } else {
    // 3. Otherwise, look for an evictable entry (ref_count == 0)
    int oldest_time = INT_MAX;
    int oldest_idx = -1;
    for (int j = 0; j < MAX_CACHED_COINS; j++) {
      if (caches[j].ref_count == 0) {
        int t = caches[j].last_sync_year * 365 + caches[j].last_sync_day;
        if (t < oldest_time) {
          oldest_time = t;
          oldest_idx = j;
        }
      }
    }
    target_idx = oldest_idx;
  }

  // 4. If no evictable entry was found, return NULL (cache full and all pinned)
  if (target_idx == -1) {
    pthread_mutex_unlock(&cache_internal_mutex);
    return NULL;
  }

  CoinCache *c = &caches[target_idx];
  pthread_mutex_t saved_mutex = c->coin_mutex;

  // Zero out the CoinCache struct EXCEPT for the coin_mutex
  memset(c, 0, sizeof(CoinCache));
  c->coin_mutex = saved_mutex;

  // Set fields
  snprintf(c->coin, sizeof(c->coin), "%s", coin);
  c->last_sync_day = -1;
  c->last_sync_year = -1;
  c->ref_count = 1;

  pthread_mutex_unlock(&cache_internal_mutex);
  return c;
}

void safe_cache_release(CoinCache *c) {
  if (!c) return;
  pthread_mutex_lock(&cache_internal_mutex);
  if (c->ref_count > 0) {
    c->ref_count--;
  }
  pthread_mutex_unlock(&cache_internal_mutex);
}

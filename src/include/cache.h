#ifndef CACHE_H
#define CACHE_H

#include <pthread.h>
#include <string.h>
#include <time.h>

#include "safe_cache.h"

static inline int should_update_cache(CoinCache *c) {
  time_t now = time(0);
  struct tm t;
  gmtime_r(&now, &t);
  return (t.tm_yday != c->last_sync_day || t.tm_year != c->last_sync_year ||
          !c->last_res.success);
}

static inline void update_cache_day(CoinCache *c) {
  time_t now = time(0);
  struct tm t;
  gmtime_r(&now, &t);
  c->last_sync_day = t.tm_yday;
  c->last_sync_year = t.tm_year;
}

#endif

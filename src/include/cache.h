#ifndef CACHE_H
#define CACHE_H

#include "models.h"
#include <time.h>
#include <string.h>

static Model* cached_model = NULL;
static char cached_coin[16] = "";
static int last_sync_window = -1;

static inline int should_update_cache(const char *coin) {
    time_t now = time(NULL);
    struct tm *t = gmtime(&now);
    int current_window = t->tm_hour / 4;

    if (strcmp(cached_coin, coin) != 0 || current_window != last_sync_window || !cached_model) {
        return 1;
    }
    return 0;
}

static inline void update_cache_metadata(const char *coin) {
    time_t now = time(NULL);
    struct tm *t = gmtime(&now);
    strcpy(cached_coin, coin);
    last_sync_window = t->tm_hour / 4;
}

#endif

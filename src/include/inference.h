#ifndef INFERENCE_H
#define INFERENCE_H

#include "models.h"
#include "cache.h"
#include "openSSL.h"
#include <sys/time.h>
#include <pthread.h>

#define BINANCE_HOST "api.binance.com"

static pthread_mutex_t cache_mutex   = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t scratch_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Static scratch buffers — reused every call, never freed, no heap fragmentation. */
static uint8_t s_model_buf[32768];
static Kline   s_kline_buf[SEQ_LEN + 2];

static inline int fetch_binance_data(const char *symbol, Kline *out, int limit) {
    SSLConnection c = create_ssl_connection(BINANCE_HOST);
    if (!c.ssl) return 0;
    char req[512], buf[8192], *p, *start, *end;
    sprintf(req, "GET /api/v3/klines?symbol=%s&interval=1d&limit=%d HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", symbol, limit, BINANCE_HOST);
    SSL_write(c.ssl, req, strlen(req));

    int n = 0, len, pos = 0, head = 0;
    while (n < limit && (len = SSL_read(c.ssl, buf + pos, sizeof(buf) - pos - 1)) > 0) {
        buf[pos += len] = 0; p = buf;
        if (!head) {
            if (!(p = strstr(buf, "\r\n\r\n"))) {
                if (pos > (int)sizeof(buf) - 512) { memmove(buf, buf + pos - 4, 4); pos = 4; }
                continue;
            }
            char *status = strstr(buf, " ");
            if (!status || atoi(status + 1) != 200) { cleanup_ssl_connection(c); return 0; }
            p += 4; head = 1;
        }
        while (n < limit && (start = strstr(p, "["))) {
            if (start[1] == '[') { p = start + 1; continue; }
            if (!(end = strstr(start, "]"))) break;
            char s[32];
            if (sscanf(start + 1, "%lld,\"%*[^\"]\",\"%*[^\"]\",\"%*[^\"]\",\"%[^\"]\"", &out[n].timestamp, s) >= 2)
                out[n++].close = atof(s);
            p = end + 1;
        }
        pos = buf + pos - p; memmove(buf, p, pos);
    }
    cleanup_ssl_connection(c);
    return n;
}

static inline int download_model_from_github(const char *coin, uint8_t *out, int sz) {
    SSLConnection c = create_ssl_connection("raw.githubusercontent.com");
    if (!c.ssl) return 0;
    char req[512], *b;
    sprintf(req, "GET /KhanhAI-VN/Test/main/%s.bin HTTP/1.0\r\nHost: raw.githubusercontent.com\r\nConnection: close\r\n\r\n", coin);
    SSL_write(c.ssl, req, strlen(req));
    int n = 0, r;
    while (n < sz - 1 && (r = SSL_read(c.ssl, out + n, sz - 1 - n)) > 0) n += r;
    out[n] = 0;
    cleanup_ssl_connection(c);
    if ((b = strstr((char*)out, "\r\n\r\n"))) {
        char *status = strstr((char*)out, " ");
        if (!status || atoi(status + 1) != 200) return 0;
        char *cl = strcasestr((char*)out, "Content-Length:");
        int expected = cl ? atoi(cl + 15) : 0;
        int h = b + 4 - (char*)out;
        int body_len = n - h;
        if (expected > 0 && body_len < expected) return 0;
        memmove(out, b + 4, body_len);
        return body_len;
    }
    return 0;
}

static inline PredictionResult run_prediction(const char *coin) {
    const char *cname = coin ? coin : "BTC";
    char symbol[32]; sprintf(symbol, "%sUSDT", cname);
    Kline latest[2] = {0};
    int n = fetch_binance_data(symbol, latest, 2);
    if (n < 2) {
        pthread_mutex_lock(&cache_mutex);
        CoinCache *cache = get_coin_cache(cname);
        PredictionResult res = cache->last_res;
        strcpy(res.coin, cache->coin);
        pthread_mutex_unlock(&cache_mutex);
        return res;
    }

    pthread_mutex_lock(&cache_mutex);
    CoinCache *cache = get_coin_cache(cname);

    if (should_update_cache(cache)) {
        int pool_idx = cache - caches;

        int days_missing = 0;
        if (cache->kline_count == SEQ_LEN + 2)
            days_missing = (int)((latest[1].timestamp - cache->klines[SEQ_LEN + 1].timestamp) / 86400000);

        int incremental = (days_missing == 1);
        int partial     = (days_missing >= 2 && days_missing <= SEQ_LEN);
        int need_model  = (cache->model == NULL);

        pthread_mutex_unlock(&cache_mutex);

        int m_len   = 0;
        int f_count = 0;

        pthread_mutex_lock(&scratch_mutex);
        if (need_model)
            m_len = download_model_from_github(cname, s_model_buf, sizeof(s_model_buf));
        if (!incremental) {
            int limit = partial ? (days_missing + 1) : (SEQ_LEN + 2);
            f_count = fetch_binance_data(symbol, s_kline_buf, limit);
        }
        pthread_mutex_unlock(&scratch_mutex);

        pthread_mutex_lock(&cache_mutex);
        cache = get_coin_cache(cname);
        if (should_update_cache(cache)) {
            if (m_len > 0) {
                model_set_pool(pool_idx);
                Model *m = load_model(s_model_buf, m_len);
                if (m) cache->model = m;
            }
            if (!cache->model) {
                PredictionResult res = {0}; strcpy(res.coin, cache->coin);
                strcpy(res.error_msg, "Model missing");
                pthread_mutex_unlock(&cache_mutex);
                return res;
            }

            if (incremental) {
                if (cache->klines[SEQ_LEN].close <= 0 || latest[1].close <= 0) {
                    pthread_mutex_unlock(&cache_mutex);
                    return (PredictionResult){.success = 0, .coin = "", .error_msg = "Invalid price data"};
                }
                memmove(cache->klines, cache->klines + 1, (SEQ_LEN + 1) * sizeof(Kline));
                cache->klines[SEQ_LEN + 1] = latest[1];
                memmove(cache->input, cache->input + 1, (SEQ_LEN - 1) * sizeof(float));
                cache->input[SEQ_LEN - 1] = logf((float)cache->klines[SEQ_LEN].close / (float)cache->klines[SEQ_LEN - 1].close);
            } else if (partial && f_count == days_missing + 1) {
                int shift = days_missing;
                memmove(cache->klines, cache->klines + shift, (SEQ_LEN + 2 - shift) * sizeof(Kline));
                for (int k = 0; k < shift; k++)
                    cache->klines[SEQ_LEN + 2 - shift + k] = s_kline_buf[k + 1];
                for (int i = 0; i < SEQ_LEN; i++) {
                    float c1 = (float)cache->klines[i].close, c2 = (float)cache->klines[i+1].close;
                    cache->input[i] = (c1 > 0 && c2 > 0) ? logf(c2 / c1) : 0;
                }
            } else if (!partial && f_count >= SEQ_LEN + 2) {
                cache->kline_count = f_count;
                memcpy(cache->klines, s_kline_buf, f_count * sizeof(Kline));
                for (int i = 0; i < SEQ_LEN; i++) {
                    float c1 = (float)cache->klines[i].close, c2 = (float)cache->klines[i+1].close;
                    cache->input[i] = (c1 > 0 && c2 > 0) ? logf(c2 / c1) : 0;
                }
            }

            if (cache->kline_count >= SEQ_LEN + 2) {
                struct timeval start, end; gettimeofday(&start, NULL);
                cache->pred_log_diff = predict(cache->model, cache->input);
                gettimeofday(&end, NULL);
                cache->last_res.success = !isnan(cache->pred_log_diff) && !isinf(cache->pred_log_diff);
                cache->last_res.inference_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
                cache->last_res.pred_change_pct = (expf(cache->pred_log_diff) - 1.0f) * 100.0f;
                time_t pred_ts = (n == 2 ? latest[1].timestamp : cache->klines[cache->kline_count-1].timestamp) / 1000 + 86400;
                struct tm t_pred;
                gmtime_r(&pred_ts, &t_pred);
                strftime(cache->last_res.date, sizeof(cache->last_res.date), "%Y-%m-%d", &t_pred);
                update_cache_day(cache);
            } else {
                PredictionResult res = {0}; strcpy(res.coin, cache->coin);
                sprintf(res.error_msg, "Data error (%d)", cache->kline_count);
                pthread_mutex_unlock(&cache_mutex);
                return res;
            }

        }
    }

    PredictionResult res = cache->last_res;
    strcpy(res.coin, cache->coin);
    res.last_price = (float)latest[1].close;
    float yesterday_close = (float)latest[0].close;
    res.change_pct = (yesterday_close > 1e-9) ? ((res.last_price - yesterday_close) / yesterday_close) * 100.0f : 0;
    res.pred_price = res.last_price * expf(cache->pred_log_diff);
    res.trend = (res.pred_price > res.last_price) ? 1 : 0;

    pthread_mutex_unlock(&cache_mutex);
    return res;
}


#endif

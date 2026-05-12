#ifndef INFERENCE_H
#define INFERENCE_H

#include "models.h"
#include "cache.h"
#include "openSSL.h"
#include <sys/time.h>

#define BINANCE_HOST "api.binance.com"

static inline int fetch_binance_data(const char *symbol, Kline *out, int limit) {
    SSLConnection c = create_ssl_connection(BINANCE_HOST);
    char req[256], *p, s[32];
    char res[72 * 1024];
    int n = 0;
    if (!c.ssl) return 0;
    sprintf(req, "GET /api/v3/klines?symbol=%s&interval=1d&limit=%d HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", symbol, limit, BINANCE_HOST);
    int len = send_http_request(&c, req, res, sizeof(res));
    if (len > 0 && (p = strstr(res, "\r\n\r\n")))
        for (p += 4; n < limit && (p = strstr(p, "[")); ) {
            if (p[1] == '[') { p++; continue; }
            if (sscanf(++p, "%lld,\"%*[^\"]\",\"%*[^\"]\",\"%*[^\"]\",\"%[^\"]\"", &out[n].timestamp, s) >= 2)
                out[n++].close = atof(s);
            if (!(p = strstr(p, "]"))) break;
        }
    cleanup_ssl_connection(c);
    return n;
}

static inline Model* download_model_from_github(const char *coin) {
    SSLConnection c = create_ssl_connection("raw.githubusercontent.com");
    char req[256], *b;
    char res[18432];
    int n = 0;
    Model *m = NULL;
    if (!c.ssl) return NULL;
    sprintf(req, "GET /KhanhAI-VN/Test/main/%s.bin HTTP/1.0\r\nHost: raw.githubusercontent.com\r\nConnection: close\r\n\r\n", coin);
    n = send_http_request(&c, req, res, sizeof(res));
    if (n > 0 && (b = strstr(res, "\r\n\r\n")))
        m = load_model((uint8_t*)(b + 4), n - (b + 4 - res));
    cleanup_ssl_connection(c);
    return m;
}

static inline PredictionResult run_prediction(const char *coin) {
    CoinCache *cache = get_coin_cache(coin ? coin : "BTC");
    
    char symbol[32]; sprintf(symbol, "%sUSDT", cache->coin);
    Kline latest[2];
    int n = fetch_binance_data(symbol, latest, 2);

    if (should_update_cache(cache)) {
        Model *m = download_model_from_github(cache->coin);
        if (m) cache->model = m;
        if (!cache->model) {
            PredictionResult res = {0}; strcpy(res.coin, cache->coin);
            strcpy(res.error_msg, "Model download failed"); return res;
        }

        if (cache->kline_count == SEQ_LEN + 2 && n == 2 && latest[0].timestamp == cache->klines[SEQ_LEN + 1].timestamp) {
            memmove(cache->klines, cache->klines + 1, (SEQ_LEN + 1) * sizeof(Kline));
            cache->klines[SEQ_LEN + 1] = latest[1];
            memmove(cache->input, cache->input + 1, (SEQ_LEN - 1) * sizeof(float));
            cache->input[SEQ_LEN - 1] = logf((float)cache->klines[SEQ_LEN].close) - logf((float)cache->klines[SEQ_LEN - 1].close);
        } else {
            cache->kline_count = fetch_binance_data(symbol, cache->klines, SEQ_LEN + 2);
            if (cache->kline_count >= SEQ_LEN + 1)
                for (int i = 0; i < SEQ_LEN; i++) 
                    cache->input[i] = logf((float)cache->klines[i+1].close) - logf((float)cache->klines[i].close);
        }

        if (cache->kline_count >= SEQ_LEN + 2) {
            struct timeval start, end; gettimeofday(&start, NULL);
            cache->pred_log_diff = predict(cache->model, cache->input);
            gettimeofday(&end, NULL);
            
            cache->last_res.success = 1;
            cache->last_res.inference_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
            cache->last_res.pred_change_pct = (expf(cache->pred_log_diff) - 1.0f) * 100.0f;
            time_t pred_ts = latest[1].timestamp / 1000 + 86400;
            strftime(cache->last_res.date, sizeof(cache->last_res.date), "%Y-%m-%d", gmtime(&pred_ts));
            update_cache_day(cache);
        } else {
            PredictionResult res = {0}; strcpy(res.coin, cache->coin);
            sprintf(res.error_msg, "Insufficient data (%d/%d)", cache->kline_count, SEQ_LEN + 2);
            return res;
        }
    }

    PredictionResult res = cache->last_res;
    strcpy(res.coin, cache->coin);
    if (n == 2) {
        res.last_price = (float)latest[1].close;
        float yesterday_close = (float)latest[0].close;
        res.change_pct = ((res.last_price - yesterday_close) / yesterday_close) * 100.0f;
        res.pred_price = res.last_price * expf(cache->pred_log_diff);
        res.trend = (res.pred_price > res.last_price) ? 1 : 0;
    }
    return res;
}

#endif

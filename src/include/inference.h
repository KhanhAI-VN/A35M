#ifndef INFERENCE_H
#define INFERENCE_H

#include "models.h"
#include "openSSL.h"
#include <sys/time.h>

#define BINANCE_HOST "api.binance.com"

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

static inline int fetch_binance_data(const char *symbol, Kline *out, int limit) {
    SSLConnection *c = create_ssl_connection(BINANCE_HOST);
    char req[256], *res, *p, s[32];
    int n = 0;
    if (!c) return 0;
    sprintf(req, "GET /api/v3/klines?symbol=%s&interval=1d&limit=%d HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", symbol, limit, BINANCE_HOST);
    if ((res = http_get(c, req, NULL)) && (p = strstr(res, "\r\n\r\n")))
        for (p += 4; n < limit && (p = strstr(p, "[")); ) {
            if (p[1] == '[') { p++; continue; }
            if (sscanf(++p, "%lld,\"%*[^\"]\",\"%*[^\"]\",\"%*[^\"]\",\"%[^\"]\"", &out[n].timestamp, s) >= 2)
                out[n++].close = atof(s);
            if (!(p = strstr(p, "]"))) break;
        }
    return free(res), cleanup_ssl_connection(c), n;
}

static inline Model* download_model_from_github(const char *coin) {
    SSLConnection *c = create_ssl_connection("raw.githubusercontent.com");
    char req[256], *res, *b;
    int n = 0;
    Model *m = NULL;
    if (!c) return NULL;
    sprintf(req, "GET /KhanhAI-VN/Test/main/%s.bin HTTP/1.0\r\nHost: raw.githubusercontent.com\r\nConnection: close\r\n\r\n", coin);
    if ((res = http_get(c, req, &n)) && (b = strstr(res, "\r\n\r\n")))
        m = load_model((uint8_t*)(b + 4), n - (b + 4 - res));
    return free(res), cleanup_ssl_connection(c), m;
}

static inline PredictionResult run_prediction(const char *coin) {
    PredictionResult res = {0};
    strcpy(res.coin, coin);
    res.success = 0;

    Model *model = download_model_from_github(coin);
    if (!model) {
        strcpy(res.error_msg, "Failed to download model from GitHub.");
        return res;
    }

    Kline klines[SEQ_LEN + 2];
    char symbol[32];
    sprintf(symbol, "%sUSDT", coin);
    int count = fetch_binance_data(symbol, klines, SEQ_LEN + 2);
    if (count < SEQ_LEN + 2) {
        sprintf(res.error_msg, "Insufficient data (%d/%d)", count, SEQ_LEN + 2);
        return res;
    }

    float input[SEQ_LEN];
    for (int i = 0; i < SEQ_LEN; i++) 
        input[i] = logf((float)klines[i+1].close) - logf((float)klines[i].close);

    struct timeval start, end;
    gettimeofday(&start, NULL);
    float pred_log_diff = predict(model, input);
    gettimeofday(&end, NULL);

    res.last_price = (float)klines[count-1].close;
    float prev_price = (float)klines[count-2].close;
    res.pred_price = res.last_price * expf(pred_log_diff);
    res.change_pct = ((res.last_price - prev_price) / prev_price) * 100.0f;
    res.pred_change_pct = (expf(pred_log_diff) - 1.0f) * 100.0f;
    res.trend = (res.pred_price > res.last_price) ? 1 : 0;
    res.inference_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    
    time_t pred_ts = klines[count-1].timestamp / 1000 + 86400;
    strftime(res.date, sizeof(res.date), "%Y-%m-%d", gmtime(&pred_ts));

    res.success = 1;
    return res;
}

#endif

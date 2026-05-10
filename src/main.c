#include "include/models.h"
#include "include/openSSL.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#define HOST "api.binance.com"

int fetch_binance_data(const char *symbol, Kline *out, int limit) {
    SSLConnection *c = create_ssl_connection(HOST);
    char req[256], *res, *p, s[32];
    int n = 0;
    if (!c) return 0;
    sprintf(req, "GET /api/v3/klines?symbol=%s&interval=1d&limit=%d HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", symbol, limit, HOST);
    if ((res = http_get(c, req, NULL)) && (p = strstr(res, "\r\n\r\n")))
        for (p += 4; n < limit && (p = strstr(p, "[")); ) {
            if (p[1] == '[') { p++; continue; }
            if (sscanf(++p, "%lld,\"%*[^\"]\",\"%*[^\"]\",\"%*[^\"]\",\"%[^\"]\"", &out[n].timestamp, s) >= 2)
                out[n++].close = atof(s);
            if (!(p = strstr(p, "]"))) break;
        }
    return free(res), cleanup_ssl_connection(c), n;
}

Model* download_model_from_github(const char *coin) {
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

int main(int argc, char **argv) {
    const char *coin = argc > 1 ? argv[1] : "BTC";
    printf("Loading %s model and data...\n", coin);

    Model *model = download_model_from_github(coin);
    if (!model) return fprintf(stderr, "Failed to load model.\n"), 1;

    Kline klines[SEQ_LEN + 2];
    char symbol[32];
    sprintf(symbol, "%sUSDT", coin);
    int count = fetch_binance_data(symbol, klines, SEQ_LEN + 2);
    if (count < SEQ_LEN + 2) return printf("Insufficient data (%d/%d)\n", count, SEQ_LEN + 2), 1;

    float input[SEQ_LEN];
    for (int i = 0; i < SEQ_LEN; i++) input[i] = logf((float)klines[i+1].close) - logf((float)klines[i].close);

    struct timeval start, end;
    gettimeofday(&start, NULL);
    float pred_log_diff = predict(model, input);
    gettimeofday(&end, NULL);

    float last = (float)klines[count-1].close, prev = (float)klines[count-2].close;
    float pred_price = last * expf(pred_log_diff);
    time_t pred_ts = klines[count-1].timestamp / 1000 + 86400;
    char pred_date[32];
    strftime(pred_date, sizeof(pred_date), "%Y-%m-%d", gmtime(&pred_ts));

    printf("\n========================================\n"
           "Asset: %s\nPrediction Day: %s (UTC)\n"
           "----------------------------------------\n"
           "Forecast: %s (%+.4f%%)\n"
           "Real-time: %+.4f%% (Price: %.2f)\n"
           "Inference: %.2f ms\n"
           "========================================\n",
           coin, pred_date, (pred_price > last) ? "UP" : "DOWN", (expf(pred_log_diff) - 1) * 100,
           ((last - prev) / prev) * 100, last,
           (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0);

    free_model(model);
    return 0;
}

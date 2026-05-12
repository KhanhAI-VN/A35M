#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include "include/inference.h"

static char html[8192], css[8192];

static enum MHD_Result send_res(struct MHD_Connection *c, const char *body, int code, const char *type) {
    struct MHD_Response *r = MHD_create_response_from_buffer(strlen(body), (void*)body, MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(r, "Content-Type", type);
    MHD_add_response_header(r, "Access-Control-Allow-Origin", "*");
    enum MHD_Result ret = MHD_queue_response(c, code, r);
    MHD_destroy_response(r);
    return ret;
}

static void load(const char *p, char *b) {
    FILE *f = fopen(p, "r");
    if (f) { b[fread(b, 1, 8191, f)] = 0; fclose(f); }
}

static enum MHD_Result handler(void *cls, struct MHD_Connection *c, const char *url, const char *meth, const char *v, const char *data, size_t *s, void **ptr) {
    if (!strcmp(url, "/api/predict")) {
        const char *coin = MHD_lookup_connection_value(c, MHD_GET_ARGUMENT_KIND, "coin");
        PredictionResult r = run_prediction(coin ? coin : "BTC");
        char json[256];
        snprintf(json, 256, "{\"success\":%s,\"price\":%.2f,\"change\":%.2f,\"trend\":\"%s\"}", 
                 r.success ? "true" : "false", r.last_price, r.change_pct, r.trend ? "UP" : "DOWN");
        return send_res(c, json, 200, "application/json");
    }
    int is_css = !strcmp(url, "/web.css");
    char *buf = is_css ? css : html;
    return buf[0] ? send_res(c, buf, 200, is_css ? "text/css" : "text/html") : send_res(c, "404", 404, "text/plain");
}

int main(int argc, char **argv) {
    const char *coins[] = {"BTC", "ETH", "SOL", "BNB", "XRP"};
    if (argc > 1) {
        printf("\n  ASSET       PRICE           PRED     CHANGE\n  ───────────────────────────────────────────\n");
        for (int i = 0; i < 5; i++) {
            if (strcmp(argv[1], "ALL") && strcmp(argv[1], coins[i])) continue;
            PredictionResult r = run_prediction(coins[i]);
            if (r.success) printf("  %-10s  $%-13.2f  %-7s (%+.2f%%)  %+.2f%%\n", coins[i], r.last_price, r.trend ? "UP" : "DOWN", r.pred_change_pct, r.change_pct);
        }
        printf("  ───────────────────────────────────────────\n\n");
        return 0;
    }
    load("src/web/web.html", html); load("src/web/web.css", css);
    struct MHD_Daemon *d = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, 8080, 0, 0, &handler, 0, MHD_OPTION_END);
    if (!d) return 1;
    printf("A35M Dashboard: http://localhost:8080\nPress Enter to stop.\n");
    getchar(); MHD_stop_daemon(d); return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include "include/inference.h"

#define PORT 8080

static enum MHD_Result send_res(struct MHD_Connection *c, const char *body, int code, const char *type) {
    struct MHD_Response *r = MHD_create_response_from_buffer(strlen(body), (void*)body, MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(r, "Content-Type", type);
    MHD_add_response_header(r, "Access-Control-Allow-Origin", "*");
    enum MHD_Result ret = MHD_queue_response(c, code, r);
    MHD_destroy_response(r);
    return ret;
}

static enum MHD_Result handler(void *cls, struct MHD_Connection *c, const char *url, const char *meth, const char *v, const char *data, size_t *s, void **ptr) {
    if (strcmp(url, "/api/predict") == 0) {
        const char *coin = MHD_lookup_connection_value(c, MHD_GET_ARGUMENT_KIND, "coin");
        PredictionResult r = run_prediction(coin ? coin : "BTC");
        char json[256];
        snprintf(json, sizeof(json), "{\"success\":%s,\"price\":%.2f,\"change\":%.2f,\"trend\":\"%s\"}", 
                 r.success ? "true" : "false", r.last_price, r.change_pct, r.trend ? "UP" : "DOWN");
        return send_res(c, json, MHD_HTTP_OK, "application/json");
    }

    const char *file_path = "src/web/web.html";
    const char *mime = "text/html";

    if (strcmp(url, "/web.css") == 0) {
        file_path = "src/web/web.css";
        mime = "text/css";
    }

    FILE *f = fopen(file_path, "r");
    if (!f) return send_res(c, "404 Not Found", MHD_HTTP_NOT_FOUND, "text/plain");
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(sz + 1);
    if (fread(buf, sz, 1, f) != 1 && sz > 0) { /* Handle read error */ }
    buf[sz] = 0;
    fclose(f);
    enum MHD_Result ret = send_res(c, buf, MHD_HTTP_OK, mime);
    free(buf);
    return ret;
}

int main(int argc, char **argv) {
    if (argc > 1) {
        PredictionResult r = run_prediction(argv[1]);
        if (r.success) {
            char chg[16];
            snprintf(chg, sizeof(chg), "%+.2f%%", r.change_pct);
            printf("┌───────────┬───────────────┬───────────┬────────────────┐\n"
                   "│ %-10s│ Now: %8.2f │ Pred: %-4s │ Change: %6s │\n"
                   "└───────────┴───────────────┴───────────┴────────────────┘\n",
                   argv[1], r.last_price, r.trend ? "UP" : "DOWN", chg);
        }
        return 0;
    }
    struct MHD_Daemon *d = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL, &handler, NULL, MHD_OPTION_END);
    if (!d) return 1;
    printf("A35M Terminal: http://localhost:%d\nPress Enter to stop.\n", PORT);
    getchar();
    MHD_stop_daemon(d);
    return 0;
}

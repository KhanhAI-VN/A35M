#ifndef OPENSSL_H
#define OPENSSL_H

#include <openssl/ssl.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>

typedef struct { SSL *ssl; int sock; SSL_CTX *ctx; SSL_SESSION *session; } SSLConnection;

typedef struct {
    SSL_SESSION *s; time_t t;
    char ip[16]; time_t ip_t;
} HostSession;

static HostSession sessions[2] = {{0}};
static const char *hosts[2] = {"api.binance.com", "raw.githubusercontent.com"};

static inline void cleanup_ssl_connection(SSLConnection c) {
    if (c.session) SSL_SESSION_free(c.session);
    if (c.ssl) SSL_free(c.ssl);
    if (c.sock >= 0) close(c.sock);
}

static inline SSLConnection create_ssl_connection(const char *h) {
    static SSL_CTX *ctx = NULL;
    if (!ctx) { SSL_library_init(); ctx = SSL_CTX_new(TLS_client_method()); }
    int s = socket(AF_INET, SOCK_STREAM, 0);
    struct timeval tv = {5, 0};
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    struct sockaddr_in a = {AF_INET, htons(443)};

    int i = strcmp(h, hosts[0]) ? 1 : 0;
    if (!sessions[i].ip[0] || difftime(time(NULL), sessions[i].ip_t) > 300) {
        struct hostent *he = gethostbyname(h);
        if (he) { inet_ntop(AF_INET, he->h_addr, sessions[i].ip, 16); sessions[i].ip_t = time(NULL); }
        else if (!sessions[i].ip[0]) return close(s), (SSLConnection){0};
    }
    struct in_addr ia; inet_pton(AF_INET, sessions[i].ip, &ia);
    memcpy(&a.sin_addr, &ia, sizeof(ia));
    if (connect(s, (struct sockaddr*)&a, sizeof(a)) < 0) return close(s), (SSLConnection){0};

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, s);
    SSL_set_tlsext_host_name(ssl, h);
    if (sessions[i].s && difftime(time(NULL), sessions[i].t) <= 300) SSL_set_session(ssl, sessions[i].s);

    int r = SSL_connect(ssl);
    if (r <= 0) {
        SSL_free(ssl); ssl = SSL_new(ctx);
        SSL_set_fd(ssl, s); SSL_set_tlsext_host_name(ssl, h);
        r = SSL_connect(ssl);
        if (r <= 0) { close(s); return (SSLConnection){0}; }
    }

    if (SSL_session_reused(ssl)) sessions[i].t = time(NULL);
    else { if (sessions[i].s) SSL_SESSION_free(sessions[i].s); sessions[i].s = SSL_get1_session(ssl); sessions[i].t = time(NULL); }

    return (SSLConnection){ssl, s, ctx, SSL_get1_session(ssl)};
}

static inline int send_http_request(SSLConnection *c, const char *req, char *res, size_t sz) {
    SSL_write(c->ssl, req, strlen(req));
    int t = 0, n;
    while (t < (int)sz - 1 && (n = SSL_read(c->ssl, res + t, sz - 1 - t)) > 0) t += n;
    return res[t] = 0, t;
}

#endif
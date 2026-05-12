#ifndef OPENSSL_H
#define OPENSSL_H

#include <openssl/ssl.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

typedef struct { SSL *ssl; int sock; SSL_CTX *ctx; } SSLConnection;

static inline void cleanup_ssl_connection(SSLConnection c) {
    if (c.ssl) SSL_free(c.ssl);
    if (c.sock >= 0) close(c.sock);
    if (c.ctx) SSL_CTX_free(c.ctx);
}

static inline SSLConnection create_ssl_connection(const char *h) {
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    struct hostent *he = gethostbyname(h);
    int s = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in a = {AF_INET, htons(443)};
    if (he) memcpy(&a.sin_addr, he->h_addr, he->h_length);
    if (!he || s < 0 || connect(s, (struct sockaddr*)&a, sizeof(a)) < 0) 
        return close(s), SSL_CTX_free(ctx), (SSLConnection){0};

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, s);
    SSL_set_tlsext_host_name(ssl, h);
    return SSL_connect(ssl) > 0 ? (SSLConnection){ssl, s, ctx} : 
           (cleanup_ssl_connection((SSLConnection){ssl, s, ctx}), (SSLConnection){0});
}

static inline int send_http_request(SSLConnection *c, const char *req, char *res, size_t sz) {
    SSL_write(c->ssl, req, strlen(req));
    int t = 0, n;
    while (t < (int)sz - 1 && (n = SSL_read(c->ssl, res + t, sz - 1 - t)) > 0) t += n;
    return res[t] = 0, t;
}

#endif
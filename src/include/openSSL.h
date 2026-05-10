#ifndef OPENSSL_H
#define OPENSSL_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { SSL *ssl; int sock; SSL_CTX *ctx; } SSLConnection;

static inline SSLConnection* create_ssl_connection(const char *host) {
    SSL_library_init();
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    struct hostent *he = gethostbyname(host);
    if (!he) return SSL_CTX_free(ctx), NULL;
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(443) };
    memcpy(&addr.sin_addr.s_addr, he->h_addr_list[0], he->h_length);
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) return close(sock), SSL_CTX_free(ctx), NULL;
    
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
    SSL_set_tlsext_host_name(ssl, host);
    SSL_connect(ssl);
    
    SSLConnection *conn = malloc(sizeof(SSLConnection));
    *conn = (SSLConnection){ssl, sock, ctx};
    return conn;
}

static inline void cleanup_ssl_connection(SSLConnection *conn) {
    if (!conn) return;
    SSL_free(conn->ssl);
    close(conn->sock);
    SSL_CTX_free(conn->ctx);
    free(conn);
}

static inline int send_http_request(SSLConnection *conn, const char *req, char *res, size_t size) {
    SSL_write(conn->ssl, req, strlen(req));
    int total = 0, n;
    while (total < (int)size - 1 && (n = SSL_read(conn->ssl, res + total, size - 1 - total)) > 0) total += n;
    res[total] = '\0';
    return total;
}

static inline char* http_get(SSLConnection *conn, const char *req, int *out_len) {
    int cap = 4096, len = 0, n;
    char *res = malloc(cap);
    SSL_write(conn->ssl, req, strlen(req));
    while (res && (n = SSL_read(conn->ssl, res + len, cap - 1 - len)) > 0)
        if ((len += n) >= cap - 1) res = realloc(res, cap *= 2);
    if (res) res[len] = 0;
    if (out_len) *out_len = len;
    return res;
}


#endif // OPENSSL_H
#ifndef OPENSSL_H
#define OPENSSL_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <pthread.h>


typedef struct { SSL *ssl; int sock; } SSLConnection;

typedef struct {
    SSL_SESSION *s; time_t t;
    char ip[16]; time_t ip_t;
} HostSession;

static HostSession sessions[3] = {{0}};
static const char *hosts[3] = {"api.binance.com", "raw.githubusercontent.com", "api.github.com"};
static pthread_mutex_t ssl_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_once_t ssl_init_once = PTHREAD_ONCE_INIT;
static SSL_CTX *ssl_ctx = NULL;

static int get_host_index(const char *h) {
    for (int i = 0; i < 3; i++) if (!strcmp(h, hosts[i])) return i;
    return 0;
}

static int new_session_cb(SSL *s, SSL_SESSION *sess) {
    const char *h = SSL_get_servername(s, TLSEXT_NAMETYPE_host_name);
    if (!h) return 0;
    int i = get_host_index(h);

    pthread_mutex_lock(&ssl_mutex);
    if (sessions[i].s) SSL_SESSION_free(sessions[i].s);
    sessions[i].s = sess; sessions[i].t = time(NULL);
    pthread_mutex_unlock(&ssl_mutex);
    return 1;
}

static void init_ssl_library(void) {
    OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);
    ssl_ctx = SSL_CTX_new(TLS_client_method());
    if (ssl_ctx) {
        SSL_CTX_set_default_verify_paths(ssl_ctx);
        SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, NULL);
        SSL_CTX_set_session_cache_mode(ssl_ctx, SSL_SESS_CACHE_CLIENT);
        SSL_CTX_sess_set_new_cb(ssl_ctx, new_session_cb);
    }
}

static inline void cleanup_ssl_connection(SSLConnection c) {
    if (c.ssl) { SSL_shutdown(c.ssl); SSL_free(c.ssl); }
    if (c.sock >= 0) close(c.sock); 
}

static inline SSLConnection create_ssl_connection(const char *h) {
    pthread_once(&ssl_init_once, init_ssl_library);
    int i = get_host_index(h);

    for (int retry = 0; retry <= 1; retry++) {
        int s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0) return (SSLConnection){NULL, -1};

        struct timeval tv = {10, 0}; 
        setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        struct sockaddr_in a = {AF_INET, htons(443), {0}, {0}};

        pthread_mutex_lock(&ssl_mutex);
        time_t now = time(NULL);
        if (!sessions[i].ip[0] || (now - sessions[i].ip_t) > 3600) {
            pthread_mutex_unlock(&ssl_mutex);
            struct addrinfo hints = {0}, *res; hints.ai_family = AF_INET;
            if (getaddrinfo(h, NULL, &hints, &res) == 0) {
                pthread_mutex_lock(&ssl_mutex);
                inet_ntop(AF_INET, &((struct sockaddr_in *)res->ai_addr)->sin_addr, sessions[i].ip, 16);
                sessions[i].ip_t = time(NULL);
                pthread_mutex_unlock(&ssl_mutex);
                freeaddrinfo(res);
            } else {
                pthread_mutex_lock(&ssl_mutex);
                if (!sessions[i].ip[0]) {
                    pthread_mutex_unlock(&ssl_mutex); close(s); return (SSLConnection){NULL, -1};
                }
                pthread_mutex_unlock(&ssl_mutex);
            }
            pthread_mutex_lock(&ssl_mutex);
        }
        struct in_addr ia; inet_pton(AF_INET, sessions[i].ip, &ia);
        memcpy(&a.sin_addr, &ia, sizeof(ia));

        SSL_SESSION *sess = sessions[i].s;
        if (sess) {
            if ((now - sessions[i].t) > 86400) {
                SSL_SESSION_free(sessions[i].s); sessions[i].s = sess = NULL;
            } else {
                SSL_SESSION_up_ref(sess);
            }
        }
        pthread_mutex_unlock(&ssl_mutex);

        if (connect(s, (struct sockaddr*)&a, sizeof(a)) < 0) {
            close(s);
            if (sess) SSL_SESSION_free(sess);
            pthread_mutex_lock(&ssl_mutex);
            sessions[i].ip[0] = 0;
            pthread_mutex_unlock(&ssl_mutex);
            continue;
        }

        if (!ssl_ctx) { close(s); if (sess) SSL_SESSION_free(sess); return (SSLConnection){NULL, -1}; }
        SSL *ssl = SSL_new(ssl_ctx);
        if (!ssl) { close(s); if (sess) SSL_SESSION_free(sess); continue; }
        SSL_set_fd(ssl, s);
        SSL_set_tlsext_host_name(ssl, h);
        if (sess) { SSL_set_session(ssl, sess); SSL_SESSION_free(sess); sess = NULL; }

        int ret = SSL_connect(ssl);
        if (ret <= 0) {
            int ssl_err = SSL_get_error(ssl, ret);
            fprintf(stderr, "SSL connect error (code %d) for %s\n", ssl_err, h);
            SSL_free(ssl);
            pthread_mutex_lock(&ssl_mutex);
            if (sessions[i].s) { SSL_SESSION_free(sessions[i].s); sessions[i].s = NULL; }
            pthread_mutex_unlock(&ssl_mutex);
            close(s); continue; 
        }

        if (SSL_get_verify_result(ssl) != X509_V_OK) {
            fprintf(stderr, "SSL verify error for %s\n", h);
            SSL_free(ssl); close(s); return (SSLConnection){NULL, -1};
        }

        if (SSL_session_reused(ssl)) {
            pthread_mutex_lock(&ssl_mutex);
            sessions[i].t = time(NULL);
            pthread_mutex_unlock(&ssl_mutex);
        }

        return (SSLConnection){ssl, s};
    }

    return (SSLConnection){NULL, -1};
}

static inline int send_http_request(SSLConnection *c, const char *req, char *res, size_t sz) {
    if (!c->ssl) return -1;
    int r = SSL_write(c->ssl, req, (int)strlen(req));
    if (r <= 0) return -1;
    size_t t = 0;
    int n;
    while (t < sz - 1 && (n = SSL_read(c->ssl, res + t, (int)(sz - 1 - t))) > 0) t += (size_t)n;
    res[t] = 0;
    return (int)t;
}

#endif
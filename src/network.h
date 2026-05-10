#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/random.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/net_sockets.h>

// Branch prediction macros
#ifndef LIKELY
#if defined(__GNUC__) || defined(__clang__)
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#endif
#endif

// Branchless SSL_RETRYABLE
#define SSL_RETRYABLE(rc)                               \
    ((int)((~(((rc ^ MBEDTLS_ERR_SSL_WANT_READ) |       \
               -(rc ^ MBEDTLS_ERR_SSL_WANT_READ)) &     \
              ((rc ^ MBEDTLS_ERR_SSL_WANT_WRITE) |      \
               -(rc ^ MBEDTLS_ERR_SSL_WANT_WRITE)))) >> \
           31) &                                        \
      1)

// Memory buffer for HTTP response
typedef struct {
    char *data;
    size_t size;
} MemoryBuffer;

// Global SSL context for reuse
typedef struct {
    mbedtls_net_context net;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_ctr_drbg_context ctr_drbg;
    int initialized;
} SSLContext;

static SSLContext g_ssl = {0};

// Custom entropy function using getrandom()
static int custom_entropy_func(void *data, unsigned char *output, size_t len) {
    (void)data;
    ssize_t ret = getrandom(output, len, GRND_NONBLOCK);
    if (UNLIKELY(ret < 0))
        return -1;
    ssize_t diff = ret ^ (ssize_t)len;
    return (int)((intptr_t)(diff | -diff) >> (sizeof(intptr_t) * 8 - 1));
}

static inline int ssl_init(void) {
    if (g_ssl.initialized)
        return 0;
        
    mbedtls_net_init(&g_ssl.net);
    mbedtls_ssl_init(&g_ssl.ssl);
    mbedtls_ssl_config_init(&g_ssl.conf);
    mbedtls_ctr_drbg_init(&g_ssl.ctr_drbg);

    const char *pers = "binance_client";
    int ret = mbedtls_ctr_drbg_seed(&g_ssl.ctr_drbg, custom_entropy_func, NULL,
                              (const unsigned char *)pers, strlen(pers));
    if (UNLIKELY(ret != 0))
        return -1;

    ret = mbedtls_ssl_config_defaults(&g_ssl.conf, MBEDTLS_SSL_IS_CLIENT,
                                      MBEDTLS_SSL_TRANSPORT_STREAM,
                                      MBEDTLS_SSL_PRESET_DEFAULT);
    if (UNLIKELY(ret != 0))
        return -1;

    mbedtls_ssl_conf_rng(&g_ssl.conf, mbedtls_ctr_drbg_random, &g_ssl.ctr_drbg);
    mbedtls_ssl_conf_authmode(&g_ssl.conf, MBEDTLS_SSL_VERIFY_NONE);

    ret = mbedtls_ssl_setup(&g_ssl.ssl, &g_ssl.conf);
    if (UNLIKELY(ret != 0))
        return -1;
        
    g_ssl.initialized = 1;
    return 0;
}

static inline int ssl_connect(const char *host, const char *port) {
    int ret = ssl_init();
    if (UNLIKELY(ret != 0))
        return -1;

    if (g_ssl.initialized) {
        mbedtls_ssl_session_reset(&g_ssl.ssl);
        mbedtls_net_free(&g_ssl.net);
    }

    ret = mbedtls_net_connect(&g_ssl.net, host, port, MBEDTLS_NET_PROTO_TCP);
    if (UNLIKELY(ret != 0))
        return -1;

    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(g_ssl.net.fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    setsockopt(g_ssl.net.fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);

    mbedtls_ssl_set_bio(&g_ssl.ssl, &g_ssl.net, mbedtls_net_send,
                        mbedtls_net_recv, NULL);

    ret = mbedtls_ssl_set_hostname(&g_ssl.ssl, host);
    if (UNLIKELY(ret != 0))
        return -1;

    for (;;) {
        ret = mbedtls_ssl_handshake(&g_ssl.ssl);
        if (LIKELY(ret == 0))
            break;
        if (SSL_RETRYABLE(ret))
            continue;
        return -1;
    }

    return g_ssl.net.fd;
}

static inline int ssl_write_all(const unsigned char *buf, size_t len) {
    size_t off = 0;
    while (off < len) {
        int rc = mbedtls_ssl_write(&g_ssl.ssl, buf + off, len - off);
        if (LIKELY(rc > 0)) {
            off += (size_t)rc;
            continue;
        }
        if (SSL_RETRYABLE(rc))
            continue;
        return -1;
    }
    return 0;
}

static inline int ssl_read(unsigned char *buf, size_t len) {
    return mbedtls_ssl_read(&g_ssl.ssl, buf, len);
}

static inline void ssl_close(void) {
    mbedtls_ssl_close_notify(&g_ssl.ssl);
    mbedtls_net_free(&g_ssl.net);
}

#endif // NETWORK_H

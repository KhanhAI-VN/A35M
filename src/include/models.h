#ifndef MODELS_H
#define MODELS_H

#include <stdint.h>
#include <string.h>
#include <math.h>

#define MAX_CACHED_COINS 10
#define SEQ_LEN 365
#define D_MODEL 16
#define PATCH_LEN 30
#define STRIDE 7
#define KERNEL_SIZE 25
#define N_P ((SEQ_LEN + (STRIDE - (SEQ_LEN - PATCH_LEN) % STRIDE) % STRIDE - PATCH_LEN) / STRIDE + 1)

typedef struct { char *name; uint32_t *dims; uint8_t num_dims; float *data; uint32_t data_len; } Tensor;
typedef struct { uint32_t version, num_tensors; Tensor *tensors; } Model;
typedef struct { float mean, stdev; } RevINStats;
typedef struct { double close; long long timestamp; } Kline;

static uint8_t model_pools[MAX_CACHED_COINS][32768];
static size_t model_offs[MAX_CACHED_COINS] = {0};
static uint8_t active_pool = 0;

static inline void model_set_pool(uint8_t idx) { active_pool = idx % MAX_CACHED_COINS; }

static inline void* model_alloc(size_t sz) {
    if (sz > 32768) return NULL;
    sz = (sz + 7) & ~7;
    size_t *offs = model_offs + active_pool;
    if (*offs + sz > sizeof(model_pools[0])) return NULL;
    uint8_t *p = model_pools[active_pool] + *offs;
    *offs += sz;
    return p;
}

static inline Model* load_model(const uint8_t *b, size_t sz) {
    if (sz < 12 || memcmp(b, "A35M", 4)) return NULL;
    model_offs[active_pool] = 0;
#define _CHECK(n) if ((size_t)(p - b) + (n) > sz) { model_offs[active_pool] = 0; return NULL; }
#define _ALLOC(ptr, n) do { if (!((ptr) = model_alloc(n))) { model_offs[active_pool] = 0; return NULL; } } while(0)
    Model *m; _ALLOC(m, sizeof(Model));
    const uint8_t *p = b + 8;
    _CHECK(4); memcpy(&m->num_tensors, p, 4); p += 4;
    if (m->num_tensors > 100) { model_offs[active_pool] = 0; return NULL; } // Sanity check
    _ALLOC(m->tensors, sizeof(Tensor) * m->num_tensors);
    for (uint32_t i = 0; i < m->num_tensors; i++) {
        uint16_t nl; _CHECK(2); memcpy(&nl, p, 2); p += 2;
        _CHECK(nl); _ALLOC(m->tensors[i].name, nl + 1); memcpy(m->tensors[i].name, p, nl); m->tensors[i].name[nl] = 0; p += nl;
        _CHECK(1); m->tensors[i].num_dims = *p++;
        if (m->tensors[i].num_dims == 0 || m->tensors[i].num_dims > 8) { model_offs[active_pool] = 0; return NULL; }
        _CHECK(4 * m->tensors[i].num_dims); _ALLOC(m->tensors[i].dims, 4 * m->tensors[i].num_dims); 
        memcpy(m->tensors[i].dims, p, 4 * m->tensors[i].num_dims); p += 4 * m->tensors[i].num_dims;
        _CHECK(5); p++; // skip padding
        memcpy(&m->tensors[i].data_len, p, 4); p += 4;
        if (m->tensors[i].data_len == 0 || m->tensors[i].data_len > 8192) { model_offs[active_pool] = 0; return NULL; }
        _CHECK(4 * m->tensors[i].data_len); _ALLOC(m->tensors[i].data, 4 * m->tensors[i].data_len); 
        memcpy(m->tensors[i].data, p, 4 * m->tensors[i].data_len); p += 4 * m->tensors[i].data_len;
    }
#undef _CHECK
#undef _ALLOC
    return m;
}

static inline Tensor* get_t(Model *m, const char *n) {
    for (uint32_t i = 0; i < m->num_tensors; i++) if (!strcmp(m->tensors[i].name, n)) return &m->tensors[i];
    return NULL;
}

static inline Tensor* get_tp(Model *m, const char *pre, const char *post) {
    char n[128]; snprintf(n, sizeof(n), "%s%s", pre, post); return get_t(m, n);
}

static inline void revin_norm(float *x, RevINStats *s, Model *m) {
    float sum = 0, sq = 0;
    for (int i = 0; i < SEQ_LEN; i++) { sum += x[i]; sq += x[i] * x[i]; }
    s->mean = sum / SEQ_LEN; 
    float var = sq / SEQ_LEN - s->mean * s->mean;
    s->stdev = sqrtf((var > 0.0f ? var : 0.0f) + 1e-5f); 
    Tensor *w = get_t(m, "revin_layer_affine_weight"), *b = get_t(m, "revin_layer_affine_bias");
    for (int i = 0; i < SEQ_LEN; i++) x[i] = ((x[i] - s->mean) / s->stdev) * (w ? w->data[0] : 1) + (b ? b->data[0] : 0);
}

static inline float revin_denorm(float x, RevINStats *s, Model *m) {
    Tensor *w = get_t(m, "revin_layer_affine_weight"), *b = get_t(m, "revin_layer_affine_bias");
    return ((x - (b ? b->data[0] : 0)) / ((w ? w->data[0] : 1) + 1e-10f)) * s->stdev + s->mean;
}

static inline void series_decomp(float *x, float *res, float *tr) {
    for (int i = 0; i < SEQ_LEN; i++) {
        float s = 0; for (int m = 0; m < KERNEL_SIZE; m++) s += x[i - m < 0 ? 0 : i - m];
        tr[i] = s / KERNEL_SIZE; res[i] = x[i] - tr[i];
    }
}

static inline void patch_linear_forward(float *in, Model *m, const char *pre, float *out) {
    float feat[N_P * D_MODEL]; 
    float avg[D_MODEL] = {0}, hid[D_MODEL];
    Tensor *wc = get_tp(m, pre, "_patch_conv_conv_weight"), *bc = get_tp(m, pre, "_patch_conv_conv_bias");
    if (!wc || !bc) return;

    for (int p = 0; p < N_P; p++) for (int d = 0; d < D_MODEL; d++) {
        float v = bc->data[d], g = bc->data[d + D_MODEL];
        for (int k = 0; k < PATCH_LEN; k++) {
            float iv = (p * STRIDE + k < SEQ_LEN) ? in[p * STRIDE + k] : in[SEQ_LEN - 1];
            v += iv * wc->data[d * PATCH_LEN + k]; g += iv * wc->data[(d + D_MODEL) * PATCH_LEN + k];
        }
        feat[d * N_P + p] = v * (0.5f * g * (1 + erff(g * 0.70710678f)));
    }
    for (int c = 0; c < D_MODEL; c++) { 
        for (int p = 0; p < N_P; p++) avg[c] += feat[c * N_P + p]; 
        avg[c] /= N_P; 
    }
    Tensor *w1 = get_tp(m, pre, "_patch_conv_se_fc_0_weight"), *b1 = get_tp(m, pre, "_patch_conv_se_fc_0_bias"), 
           *w2 = get_tp(m, pre, "_patch_conv_se_fc_2_weight"), *b2 = get_tp(m, pre, "_patch_conv_se_fc_2_bias");
    if (!w1 || !b1 || !w2 || !b2) return;

    for (int i = 0; i < (int)w1->dims[0] && i < D_MODEL; i++) { 
        hid[i] = b1->data[i]; 
        for (int j = 0; j < D_MODEL; j++) hid[i] += avg[j] * w1->data[i * D_MODEL + j]; 
        if (hid[i] < 0) hid[i] = 0; // ReLU
    }
    for (int i = 0; i < D_MODEL; i++) {
        float sc = b2->data[i]; 
        for (int j = 0; j < (int)w1->dims[0] && j < D_MODEL; j++) sc += hid[j] * w2->data[i * w1->dims[0] + j];
        sc = 1 / (1 + expf(-sc)); // Sigmoid
        for (int p = 0; p < N_P; p++) feat[i * N_P + p] *= sc;
    }
    Tensor *wh = get_tp(m, pre, "_head_linear_weight"), *bh = get_tp(m, pre, "_head_linear_bias");
    if (!wh || !bh) return;
    out[0] = bh->data[0]; 
    for (int i = 0; i < D_MODEL * N_P; i++) out[0] += feat[i] * wh->data[i];
}

static inline float predict(Model *m, float *in) {
    // Stack-allocated buffers (~4.3KB)
    float x[SEQ_LEN], res[SEQ_LEN], tr[SEQ_LEN]; 
    float ro[1], to[1]; RevINStats s;
    memcpy(x, in, SEQ_LEN * 4); 
    revin_norm(x, &s, m); 
    series_decomp(x, res, tr);
    patch_linear_forward(res, m, "model_res", ro); 
    patch_linear_forward(tr, m, "model_trend", to);
    return revin_denorm(ro[0] + to[0], &s, m);
}

#endif
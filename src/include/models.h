#ifndef MODELS_H
#define MODELS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SEQ_LEN 365
#define D_MODEL 16
#define PATCH_LEN 30
#define STRIDE 7
#define KERNEL_SIZE 25

typedef struct { char *name; uint32_t *dims; uint8_t num_dims; float *data; uint32_t data_len; } Tensor;
typedef struct { uint32_t version, num_tensors; Tensor *tensors; } Model;
typedef struct { float mean, stdev; } RevINStats;
typedef struct { double close; long long timestamp; } Kline;

static uint8_t model_pool[18512];
static size_t model_off = 0;
static float model_workspace[2048];

static inline void* model_alloc(size_t sz) {
    void *p = model_pool + model_off;
    return (model_off += (sz + 3) & ~3) <= sizeof(model_pool) ? p : NULL;
}

static inline Model* load_model(const uint8_t *buf, size_t size) {
    if (size < 4 || memcmp(buf, "A35M", 4)) return NULL;
    model_off = 0;
    Model *m = model_alloc(sizeof(Model));
    size_t off = 8;
    memcpy(&m->num_tensors, buf + off, 4); off += 4;
    m->tensors = model_alloc(sizeof(Tensor) * m->num_tensors);
    for (uint32_t i = 0; i < m->num_tensors; i++) {
        uint16_t nl; memcpy(&nl, buf + off, 2); off += 2;
        m->tensors[i].name = model_alloc(nl + 1); memcpy(m->tensors[i].name, buf + off, nl); m->tensors[i].name[nl] = 0; off += nl;
        m->tensors[i].num_dims = buf[off++];
        m->tensors[i].dims = model_alloc(4 * m->tensors[i].num_dims); memcpy(m->tensors[i].dims, buf + off, 4 * m->tensors[i].num_dims); off += 4 * m->tensors[i].num_dims;
        off++; // skip dtype
        memcpy(&m->tensors[i].data_len, buf + off, 4); off += 4;
        m->tensors[i].data = model_alloc(4 * m->tensors[i].data_len); memcpy(m->tensors[i].data, buf + off, 4 * m->tensors[i].data_len); off += 4 * m->tensors[i].data_len;
    }
    return m;
}

static inline Tensor* get_t(Model *m, const char *n) {
    for (uint32_t i = 0; i < m->num_tensors; i++) if (!strcmp(m->tensors[i].name, n)) return &m->tensors[i];
    return NULL;
}

static inline Tensor* get_tp(Model *m, const char *pre, const char *post) {
    char n[128]; strcpy(n, pre); strcat(n, post);
    return get_t(m, n);
}

static inline void revin_norm(float *x, RevINStats *s, Model *m) {
    float sum = 0, sq = 0;
    for (int i = 0; i < SEQ_LEN; i++) { sum += x[i]; sq += x[i] * x[i]; }
    s->mean = sum / SEQ_LEN;
    s->stdev = sqrtf(sq / SEQ_LEN - s->mean * s->mean + 1e-5f);
    Tensor *w = get_t(m, "revin_layer_affine_weight"), *b = get_t(m, "revin_layer_affine_bias");
    for (int i = 0; i < SEQ_LEN; i++) x[i] = ((x[i] - s->mean) / s->stdev) * (w ? w->data[0] : 1) + (b ? b->data[0] : 0);
}

static inline float revin_denorm(float x, RevINStats *s, Model *m) {
    Tensor *w = get_t(m, "revin_layer_affine_weight"), *b = get_t(m, "revin_layer_affine_bias");
    float wv = w ? w->data[0] : 1, bv = b ? b->data[0] : 0;
    return ((x - bv) / (wv + 1e-10f)) * s->stdev + s->mean;
}

static inline void series_decomp(float *x, float *res, float *trend) {
    for (int i = 0; i < SEQ_LEN; i++) {
        float s = 0;
        for (int m = 0; m < KERNEL_SIZE; m++) s += x[i - m < 0 ? 0 : i - m];
        trend[i] = s / KERNEL_SIZE; res[i] = x[i] - trend[i];
    }
}

static inline void patch_linear_forward(float *in, Model *m, const char *pre, float *out, float *ws) {
    Tensor *wc = get_tp(m, pre, "_patch_conv_conv_weight"), *bc = get_tp(m, pre, "_patch_conv_conv_bias");
    int pad = (STRIDE - (SEQ_LEN - PATCH_LEN) % STRIDE) % STRIDE, n_p = (SEQ_LEN + pad - PATCH_LEN) / STRIDE + 1;
    float *feat = ws, *avg = ws + (n_p * D_MODEL), *hid = avg + D_MODEL;
    for (int p = 0; p < n_p; p++) {
        for (int d = 0; d < D_MODEL; d++) {
            float v = bc->data[d], g = bc->data[d + D_MODEL];
            for (int k = 0; k < PATCH_LEN; k++) {
                float iv = (p * STRIDE + k < SEQ_LEN) ? in[p * STRIDE + k] : in[SEQ_LEN - 1];
                v += iv * wc->data[d * PATCH_LEN + k]; g += iv * wc->data[(d + D_MODEL) * PATCH_LEN + k];
            }
            feat[d * n_p + p] = v * (0.5f * g * (1 + erff(g / 1.41421356f)));
        }
    }
    for (int c = 0; c < D_MODEL; c++) { avg[c] = 0; for (int p = 0; p < n_p; p++) avg[c] += feat[c * n_p + p]; avg[c] /= n_p; }
    Tensor *w1 = get_tp(m, pre, "_patch_conv_se_fc_0_weight"), *b1 = get_tp(m, pre, "_patch_conv_se_fc_0_bias");
    int inn = w1->dims[0];
    for (int i = 0; i < inn; i++) { hid[i] = b1->data[i]; for (int j = 0; j < D_MODEL; j++) hid[i] += avg[j] * w1->data[i * D_MODEL + j]; if (hid[i] < 0) hid[i] = 0; }
    Tensor *w2 = get_tp(m, pre, "_patch_conv_se_fc_2_weight"), *b2 = get_tp(m, pre, "_patch_conv_se_fc_2_bias");
    for (int i = 0; i < D_MODEL; i++) {
        float sc = b2->data[i]; for (int j = 0; j < inn; j++) sc += hid[j] * w2->data[i * inn + j];
        sc = 1 / (1 + expf(-sc)); for (int p = 0; p < n_p; p++) feat[i * n_p + p] *= sc;
    }
    Tensor *wh = get_tp(m, pre, "_head_linear_weight"), *bh = get_tp(m, pre, "_head_linear_bias");
    out[0] = bh->data[0]; for (int i = 0; i < D_MODEL * n_p; i++) out[0] += feat[i] * wh->data[i];
}

static inline float predict(Model *m, float *in) {
    float *ws = model_workspace;
    float *x = ws, *res = ws + SEQ_LEN, *tr = ws + (SEQ_LEN * 2), *sub_ws = ws + (SEQ_LEN * 3);
    float ro[1], to[1]; memcpy(x, in, SEQ_LEN * 4);
    RevINStats s; revin_norm(x, &s, m);
    series_decomp(x, res, tr);
    patch_linear_forward(res, m, "model_res", ro, sub_ws);
    patch_linear_forward(tr, m, "model_trend", to, sub_ws);
    return revin_denorm(ro[0] + to[0], &s, m);
}

#endif
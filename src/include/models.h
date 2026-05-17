#ifndef MODELS_H
#define MODELS_H

#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>

#include "../../3libs/hashmap.h"
#include "../../3libs/stb_sprintf.h"

#define MAX_CACHED_COINS 45
#define SEQ_LEN 365
#define D_MODEL 16
#define PATCH_LEN 30
#define STRIDE 7
#define KERNEL_SIZE 25
#define N_P                                                        \
  ((SEQ_LEN + (STRIDE - (SEQ_LEN - PATCH_LEN) % STRIDE) % STRIDE - \
    PATCH_LEN) /                                                   \
       STRIDE +                                                    \
   1)

typedef struct {
  char *name;
  uint32_t *dims;
  uint8_t num_dims;
  float *data;
  uint32_t data_len;
} Tensor;
typedef struct {
  uint32_t version, num_tensors;
  uint8_t pool_idx;
  Tensor *tensors;
} Model;
typedef struct {
  const char *name;
  Tensor *ptr;
} TensorEntry;
typedef struct {
  float mean, stdev;
} RevINStats;
typedef struct {
  double close;
  long long timestamp;
} Kline;

static uint8_t model_pools[MAX_CACHED_COINS][49152];
static size_t model_offs[MAX_CACHED_COINS] = {0};
static struct hashmap *model_indices[MAX_CACHED_COINS] = {NULL};
static pthread_mutex_t model_mutex = PTHREAD_MUTEX_INITIALIZER;

static uint64_t tensor_hash(const void *item, uint64_t s0, uint64_t s1) {
  return hashmap_sip(((const TensorEntry *)item)->name,
                     strlen(((const TensorEntry *)item)->name), s0, s1);
}
static int tensor_cmp(const void *a, const void *b, void *u) {
  (void)u;
  return strcmp(((const TensorEntry *)a)->name, ((const TensorEntry *)b)->name);
}

static inline void *model_alloc(uint8_t pool_idx, size_t sz) {
  pthread_mutex_lock(&model_mutex);
  if (sz > 49152) {
    pthread_mutex_unlock(&model_mutex);
    return NULL;
  }
  sz = (sz + 7) & ~7;
  uint8_t idx = pool_idx % MAX_CACHED_COINS;
  if (model_offs[idx] + sz > sizeof(model_pools[0])) {
    pthread_mutex_unlock(&model_mutex);
    return NULL;
  }
  uint8_t *p = model_pools[idx] + model_offs[idx];
  model_offs[idx] += sz;
  pthread_mutex_unlock(&model_mutex);
  return p;
}

static inline Model *load_model(uint8_t pool_idx, const uint8_t *b, size_t sz) {
  if (sz < 12 || memcmp(b, "A35M", 4)) return NULL;
  uint8_t idx = pool_idx % MAX_CACHED_COINS;
  pthread_mutex_lock(&model_mutex);
  model_offs[idx] = 0;
  if (model_indices[idx]) {
    hashmap_free(model_indices[idx]);
    model_indices[idx] = NULL;
  }
  pthread_mutex_unlock(&model_mutex);

#define _CHECK(n)                       \
  if ((size_t)(p - b) + (n) > sz) {     \
    pthread_mutex_lock(&model_mutex);   \
    model_offs[idx] = 0;                \
    pthread_mutex_unlock(&model_mutex); \
    return NULL;                        \
  }
#define _ALLOC(ptr, n)                         \
  do {                                         \
    if (!((ptr) = model_alloc(pool_idx, n))) { \
      pthread_mutex_lock(&model_mutex);        \
      model_offs[idx] = 0;                     \
      pthread_mutex_unlock(&model_mutex);      \
      return NULL;                             \
    }                                          \
  } while (0)
  Model *m;
  _ALLOC(m, sizeof(Model));
  m->pool_idx = idx;
  const uint8_t *p = b + 8;
  _CHECK(4);
  memcpy(&m->num_tensors, p, 4);
  p += 4;
  if (m->num_tensors > 100) {
    pthread_mutex_lock(&model_mutex);
    model_offs[idx] = 0;
    pthread_mutex_unlock(&model_mutex);
    return NULL;
  }
  _ALLOC(m->tensors, sizeof(Tensor) * m->num_tensors);
  for (uint32_t i = 0; i < m->num_tensors; i++) {
    uint16_t nl;
    _CHECK(2);
    memcpy(&nl, p, 2);
    p += 2;
    _CHECK(nl);
    _ALLOC(m->tensors[i].name, nl + 1);
    memcpy(m->tensors[i].name, p, nl);
    m->tensors[i].name[nl] = 0;
    p += nl;
    _CHECK(1);
    m->tensors[i].num_dims = *p++;
    if (m->tensors[i].num_dims == 0 || m->tensors[i].num_dims > 8) {
      pthread_mutex_lock(&model_mutex);
      model_offs[idx] = 0;
      pthread_mutex_unlock(&model_mutex);
      return NULL;
    }
    _CHECK(4 * m->tensors[i].num_dims);
    _ALLOC(m->tensors[i].dims, 4 * m->tensors[i].num_dims);
    memcpy(m->tensors[i].dims, p, 4 * m->tensors[i].num_dims);
    p += 4 * m->tensors[i].num_dims;
    _CHECK(5);
    p++;
    memcpy(&m->tensors[i].data_len, p, 4);
    p += 4;
    if (m->tensors[i].data_len == 0 || m->tensors[i].data_len > 8192) {
      pthread_mutex_lock(&model_mutex);
      model_offs[idx] = 0;
      pthread_mutex_unlock(&model_mutex);
      return NULL;
    }
    _CHECK(4 * m->tensors[i].data_len);
    _ALLOC(m->tensors[i].data, 4 * m->tensors[i].data_len);
    memcpy(m->tensors[i].data, p, 4 * m->tensors[i].data_len);
    p += 4 * m->tensors[i].data_len;
  }
#undef _CHECK
#undef _ALLOC

  struct hashmap *hm = hashmap_new(sizeof(TensorEntry), m->num_tensors, 0, 0,
                                   tensor_hash, tensor_cmp, NULL, NULL);
  if (hm) {
    for (uint32_t i = 0; i < m->num_tensors; i++) {
      TensorEntry e = {m->tensors[i].name, &m->tensors[i]};
      hashmap_set(hm, &e);
    }
    pthread_mutex_lock(&model_mutex);
    model_indices[idx] = hm;
    pthread_mutex_unlock(&model_mutex);
  }
  return m;
}

static inline Tensor *get_t(Model *m, const char *n) {
  TensorEntry key = {n, NULL};
  const TensorEntry *e = hashmap_get(model_indices[m->pool_idx], &key);
  return e ? e->ptr : NULL;
}

static inline Tensor *get_tp(Model *m, const char *pre, const char *post) {
  char n[128];
  stbsp_snprintf(n, sizeof(n), "%s%s", pre, post);
  return get_t(m, n);
}

static inline void revin_norm(float *restrict x, RevINStats *restrict s,
                              Model *m) {
  if (!x || !s || !m) return;
  float sum = 0, sq = 0;
  for (int i = 0; i < SEQ_LEN; i++) {
    sum += x[i];
    sq += x[i] * x[i];
  }
  s->mean = sum / SEQ_LEN;
  float var = sq / SEQ_LEN - s->mean * s->mean;
  s->stdev = sqrtf((var > 0.0f ? var : 0.0f) + 1e-5f);
  Tensor *w = get_t(m, "revin_layer_affine_weight"),
         *b = get_t(m, "revin_layer_affine_bias");
  if (!w || w->data_len < 1 || !b || b->data_len < 1) {
    for (int i = 0; i < SEQ_LEN; i++) x[i] = (x[i] - s->mean) / s->stdev;
    return;
  }
#pragma GCC ivdep
  for (int i = 0; i < SEQ_LEN; i++)
    x[i] = ((x[i] - s->mean) / s->stdev) * w->data[0] + b->data[0];
}

static inline float revin_denorm(float x, RevINStats *s, Model *m) {
  if (!s || !m) return x;
  Tensor *w = get_t(m, "revin_layer_affine_weight"),
         *b = get_t(m, "revin_layer_affine_bias");
  float weight = (w && w->data_len > 0) ? w->data[0] : 1.0f;
  float bias = (b && b->data_len > 0) ? b->data[0] : 0.0f;
  return ((x - bias) / (weight + 1e-10f)) * s->stdev + s->mean;
}

static inline void series_decomp(const float *restrict x, float *restrict res,
                                 float *restrict tr) {
  float sum = x[0] * KERNEL_SIZE;
  for (int i = 0; i < SEQ_LEN; i++) {
    tr[i] = sum / KERNEL_SIZE;
    res[i] = x[i] - tr[i];
    if (i + 1 < SEQ_LEN) {
      sum -= (i - KERNEL_SIZE + 1 < 0) ? x[0] : x[i - KERNEL_SIZE + 1];
      sum += x[i + 1];
    }
  }
}

static inline void patch_linear_forward(const float *restrict in, Model *m,
                                        const char *pre, float *restrict out) {
  float feat[N_P * D_MODEL];
  float avg[D_MODEL] = {0}, hid[D_MODEL];
  Tensor *wc = get_tp(m, pre, "_patch_conv_conv_weight"),
         *bc = get_tp(m, pre, "_patch_conv_conv_bias");
  if (!wc || !bc || wc->data_len < (2 * D_MODEL * PATCH_LEN) ||
      bc->data_len < (2 * D_MODEL))
    return;

  for (int p = 0; p < N_P; p++) {
    float patch_in[PATCH_LEN];
    for (int k = 0; k < PATCH_LEN; k++) {
      patch_in[k] =
          (p * STRIDE + k < SEQ_LEN) ? in[p * STRIDE + k] : in[SEQ_LEN - 1];
    }
    for (int d = 0; d < D_MODEL; d++) {
      float v = bc->data[d], g = bc->data[d + D_MODEL];
      float *restrict w_v = &wc->data[d * PATCH_LEN];
      float *restrict w_g = &wc->data[(d + D_MODEL) * PATCH_LEN];
#pragma GCC unroll 4
      for (int k = 0; k < PATCH_LEN; k++) {
        v += patch_in[k] * w_v[k];
        g += patch_in[k] * w_g[k];
      }
      feat[d * N_P + p] = v * (0.5f * g * (1 + erff(g * 0.70710678f)));
    }
  }
  for (int c = 0; c < D_MODEL; c++) {
    float s = 0;
    for (int p = 0; p < N_P; p++) s += feat[c * N_P + p];
    avg[c] = s / N_P;
  }
  Tensor *w1 = get_tp(m, pre, "_patch_conv_se_fc_0_weight"),
         *b1 = get_tp(m, pre, "_patch_conv_se_fc_0_bias"),
         *w2 = get_tp(m, pre, "_patch_conv_se_fc_2_weight"),
         *b2 = get_tp(m, pre, "_patch_conv_se_fc_2_bias");
  if (!w1 || !b1 || !w2 || !b2 || w1->num_dims < 2 || w2->num_dims < 2) return;

  int h_dim = (int)w1->dims[0];
  if (h_dim > D_MODEL || w1->data_len < h_dim * D_MODEL ||
      b1->data_len < h_dim || w2->data_len < D_MODEL * h_dim ||
      b2->data_len < D_MODEL)
    return;

  for (int i = 0; i < h_dim; i++) {
    hid[i] = b1->data[i];
#pragma GCC unroll 4
    for (int j = 0; j < D_MODEL; j++)
      hid[i] += avg[j] * w1->data[i * D_MODEL + j];
    if (hid[i] < 0) hid[i] = 0;
  }
  for (int i = 0; i < D_MODEL; i++) {
    float sc = b2->data[i];
#pragma GCC unroll 4
    for (int j = 0; j < h_dim; j++) sc += hid[j] * w2->data[i * h_dim + j];
    sc = 1 / (1 + expf(-sc));
    for (int p = 0; p < N_P; p++) feat[i * N_P + p] *= sc;
  }
  Tensor *wh = get_tp(m, pre, "_head_linear_weight"),
         *bh = get_tp(m, pre, "_head_linear_bias");
  if (!wh || !bh || wh->data_len < (D_MODEL * N_P) || bh->data_len < 1) return;
  out[0] = bh->data[0];
#pragma GCC ivdep
  for (int i = 0; i < D_MODEL * N_P; i++) out[0] += feat[i] * wh->data[i];
}

static inline float predict(Model *m, const float *restrict in) {
  if (!m || !in) return NAN;
  float x[SEQ_LEN], res[SEQ_LEN], tr[SEQ_LEN];
  float ro[1] = {0}, to[1] = {0};
  RevINStats s = {0};
  memcpy(x, in, SEQ_LEN * 4);
  revin_norm(x, &s, m);
  series_decomp(x, res, tr);
  patch_linear_forward(res, m, "model_res", ro);
  patch_linear_forward(tr, m, "model_trend", to);
  return revin_denorm(ro[0] + to[0], &s, m);
}

#endif
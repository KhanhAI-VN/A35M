// Auto-loaded model weights from .bin file
// Binary format: A35M v1 (produced by train_models.py)
#ifndef MODELS_H
#define MODELS_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "network.h"

#define MODEL_TOTAL_FLOATS 3852

// TỐI ƯU 1: Gộp bộ đệm mạng và bộ đệm Model thành MỘT buffer duy nhất.
// Giới hạn 20KB là đủ cho cả HTTP Header và Payload. 
// Sau khi parse, các float sẽ được dồn lên đầu mảng này.
#define SHARED_BUF_SIZE (20 * 1024)
uint8_t g_shared_buf[SHARED_BUF_SIZE] __attribute__((aligned(4)));

// Pointers into g_shared_buf
float *revin_layer_affine_weight = NULL;
float *revin_layer_affine_bias = NULL;

static float *model_trend_patch_conv_conv_weight;
static float *model_trend_patch_conv_conv_bias;
static float *model_trend_patch_conv_se_fc_0_weight;
static float *model_trend_patch_conv_se_fc_0_bias;
static float *model_trend_patch_conv_se_fc_2_weight;
static float *model_trend_patch_conv_se_fc_2_bias;
static float *model_trend_head_linear_weight;
static float *model_trend_head_linear_bias;

static float *model_res_patch_conv_conv_weight;
static float *model_res_patch_conv_conv_bias;
static float *model_res_patch_conv_se_fc_0_weight;
static float *model_res_patch_conv_se_fc_0_bias;
static float *model_res_patch_conv_se_fc_2_weight;
static float *model_res_patch_conv_se_fc_2_bias;
static float *model_res_head_linear_weight;
static float *model_res_head_linear_bias;

typedef struct {
    const char *name;
    uint16_t num_floats;
    float **ptr; 
} tensor_slot_t;

const tensor_slot_t g_slots[] = {
    {"model_res_head_linear_bias",           1,    &model_res_head_linear_bias},
    {"model_res_head_linear_weight",         784,  &model_res_head_linear_weight},
    {"model_res_patch_conv_conv_bias",       32,   &model_res_patch_conv_conv_bias},
    {"model_res_patch_conv_conv_weight",     960,  &model_res_patch_conv_conv_weight},
    {"model_res_patch_conv_se_fc_0_bias",    4,    &model_res_patch_conv_se_fc_0_bias},
    {"model_res_patch_conv_se_fc_0_weight",  64,   &model_res_patch_conv_se_fc_0_weight},
    {"model_res_patch_conv_se_fc_2_bias",    16,   &model_res_patch_conv_se_fc_2_bias},
    {"model_res_patch_conv_se_fc_2_weight",  64,   &model_res_patch_conv_se_fc_2_weight},
    {"model_trend_head_linear_bias",         1,    &model_trend_head_linear_bias},
    {"model_trend_head_linear_weight",       784,  &model_trend_head_linear_weight},
    {"model_trend_patch_conv_conv_bias",     32,   &model_trend_patch_conv_conv_bias},
    {"model_trend_patch_conv_conv_weight",   960,  &model_trend_patch_conv_conv_weight},
    {"model_trend_patch_conv_se_fc_0_bias",  4,    &model_trend_patch_conv_se_fc_0_bias},
    {"model_trend_patch_conv_se_fc_0_weight",64,   &model_trend_patch_conv_se_fc_0_weight},
    {"model_trend_patch_conv_se_fc_2_bias",  16,   &model_trend_patch_conv_se_fc_2_bias},
    {"model_trend_patch_conv_se_fc_2_weight",64,   &model_trend_patch_conv_se_fc_2_weight},
    {"revin_layer_affine_bias",              1,    &revin_layer_affine_bias},
    {"revin_layer_affine_weight",            1,    &revin_layer_affine_weight},
};

const int NUM_SLOTS = sizeof(g_slots) / sizeof(g_slots[0]);

// Read helpers (little-endian, works on ARM)
static inline uint16_t rd16(const uint8_t *p) { uint16_t v; memcpy(&v, p, 2); return v; }
static inline uint32_t rd32(const uint8_t *p) { uint32_t v; memcpy(&v, p, 4); return v; }

// TỐI ƯU 3: C implementation replaced by ARM Assembly in asm/load_model_bin.S
extern int load_model_bin(size_t len);

// Trả về 0 nếu thành công, xuất thẳng vào g_shared_buf (Không dùng malloc)
static inline int fetch_model_bin(const char *coin, size_t *out_len) {
    char path[128]; // Tối ưu Stack
    snprintf(path, sizeof(path), "/KhanhAI-VN/Test/main/%s.bin?t=%lld", coin, (long long)time(NULL));

    int fd = ssl_connect("raw.githubusercontent.com", "443");
    if (UNLIKELY(fd < 0)) return -1;

    char request[256]; // Tối ưu Stack
    snprintf(request, sizeof(request),
        "GET %s HTTP/1.1\r\n"
        "Host: raw.githubusercontent.com\r\n"
        "User-Agent: A35-predict/1.0\r\n"
        "Connection: close\r\n\r\n", path);

    if (UNLIKELY(ssl_write_all((const unsigned char *)request, strlen(request)) != 0)) {
        ssl_close();
        return -1;
    }

    size_t total = 0;
    int rc;
    // Đọc thẳng vào mảng tĩnh g_shared_buf
    while ((rc = ssl_read(g_shared_buf + total, SHARED_BUF_SIZE - total)) > 0) {
        total += (size_t)rc;
        if (total >= SHARED_BUF_SIZE) break;
    }
    ssl_close();

    if (total < 20) return -1;

    // Tìm điểm kết thúc Header (\r\n\r\n)
    uint8_t *body = NULL;
    for (size_t i = 0; i + 3 < total; i++) {
        if (g_shared_buf[i] == '\r' && g_shared_buf[i+1] == '\n' && 
            g_shared_buf[i+2] == '\r' && g_shared_buf[i+3] == '\n') {
            body = g_shared_buf + i + 4;
            break;
        }
    }
    if (!body) return -1;

    // Kiểm tra chính xác HTTP Status Code 200 ở ngay dòng đầu tiên
    if (total < 12 || memcmp(g_shared_buf + 9, "200", 3) != 0) {
        return -1;
    }

    size_t body_len = total - (size_t)(body - g_shared_buf);
    *out_len = body_len;
    
    // Dịch chuyển file bin về đầu buffer để đảm bảo alignment bộ nhớ (Aligned to 4 bytes)
    memmove(g_shared_buf, body, body_len);
    
    return 0;
}

static inline int load_model_from_github(const char *coin) {
    printf("Downloading %s model from GitHub...\n", coin);
    size_t bin_len = 0;
    
    if (fetch_model_bin(coin, &bin_len) != 0) {
        fprintf(stderr, "Download failed.\n");
        return -1;
    }

    printf("Downloaded %zu bytes. Parsing model...\n", bin_len);
    int ret = load_model_bin(bin_len);
    
    if (ret == 0) {
        printf("Model loaded successfully (%d parameters)\n", MODEL_TOTAL_FLOATS);
    } else {
        fprintf(stderr, "Failed to parse model.\n");
    }
    return ret;
}

#endif // MODELS_H
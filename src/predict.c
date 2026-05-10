#include <math.h>
#include <time.h>
#include "network.h"
#include "models.h"

#define SEQ_LEN 365
#define PATCH_LEN 30
#define STRIDE 7
#define D_MODEL 16
#define NUM_FEATURES 1
#define MAX_CANDLES 400
typedef struct {
    int total_parsed;
    int write_offset;
} ParseState;

extern int parse_close_chunk(const char *json_chunk, int days, float *out_data, ParseState *state);
extern void rotate_buffer(float *arr, int total_parsed, int days);

static int fetch_json_data(const char *symbol, int days, float *out_data) {
    time_t now = time(NULL);
    time_t start_time = now - (days + 5) * 24 * 3600;

    char url[512];
    snprintf(url, sizeof(url), "/api/v3/klines?symbol=%s&interval=1d&limit=%d&startTime=%llu000",
             symbol, days + 5, (unsigned long long)start_time * 1000);

    int fd = ssl_connect("api.binance.com", "443");
    if (UNLIKELY(fd < 0)) {
        fprintf(stderr, "SSL connect failed\n");
        return -1;
    }

    char request[1024];
    snprintf(request, sizeof(request),
        "GET %s HTTP/1.1\r\n"
        "Host: api.binance.com\r\n"
        "Connection: close\r\n"
        "\r\n", url);

    if (UNLIKELY(ssl_write_all((const unsigned char *)request, strlen(request)) != 0)) {
        fprintf(stderr, "SSL write failed\n");
        ssl_close();
        return -1;
    }

    ParseState state = {0, 0};
    int headers_passed = 0;
    int is_first_chunk = 1;
    int tail_len = 0;
    char buffer[2048];
    int received;
    
    while ((received = ssl_read((unsigned char *)buffer + tail_len, sizeof(buffer) - 1 - tail_len)) > 0) {
        int total_len = tail_len + received;
        buffer[total_len] = '\0';
        
        char *json_start = buffer;
        
        if (!headers_passed) {
            char *body = NULL;
            for (int i = 0; i + 3 < total_len; i++) {
                if (buffer[i] == '\r' && buffer[i+1] == '\n' && 
                    buffer[i+2] == '\r' && buffer[i+3] == '\n') {
                    body = buffer + i + 4;
                    break;
                }
            }

            if (body) {
                // Kiểm tra HTTP Status 200 ở dòng đầu tiên
                if (total_len > 12 && memcmp(buffer + 9, "200", 3) != 0) {
                    fprintf(stderr, "HTTP error: not 200 OK\n");
                    return -1;
                }
                headers_passed = 1;
                json_start = body;
            } else {
                tail_len = total_len;
                if (tail_len == sizeof(buffer) - 1) tail_len = 0; 
                continue; 
            }
        }
        
        if (is_first_chunk && headers_passed) {
            char *first_bracket = strchr(json_start, '[');
            if (first_bracket) {
                json_start = first_bracket + 1; // Skip global '['
                is_first_chunk = 0;
            }
        }
        
        int last_bracket = -1;
        for (int i = total_len - 1; i >= (json_start - buffer); i--) {
            if (buffer[i] == ']') {
                last_bracket = i;
                break;
            }
        }
        
        if (last_bracket != -1) {
            char temp = buffer[last_bracket + 1];
            buffer[last_bracket + 1] = '\0';
            
            parse_close_chunk(json_start, days, out_data, &state);
            
            buffer[last_bracket + 1] = temp;
            tail_len = total_len - (last_bracket + 1);
            memmove(buffer, buffer + last_bracket + 1, tail_len);
        } else {
            tail_len = total_len;
            if (tail_len >= sizeof(buffer) - 1) tail_len = 0;
        }
    }

    ssl_close();
    rotate_buffer(out_data, state.total_parsed, days);
    
    int num_candles = state.total_parsed;
    if (num_candles > days) num_candles = days;
    return num_candles;
}

extern void compute_log_returns(const float *data, int n, float out[SEQ_LEN][NUM_FEATURES]);
extern void revin_normalize(float data[SEQ_LEN][NUM_FEATURES], float *mean, float *stdev);
extern void moving_average(const float *input, float *output, int len, int kernel_size);
extern void decompose(const float input[SEQ_LEN][NUM_FEATURES], 
                      float trend[SEQ_LEN][NUM_FEATURES],
                      float residual[SEQ_LEN][NUM_FEATURES]);
extern void conv1d(const float *input, int input_len, 
                   const float *weight, const float *bias, 
                   int in_channels, int out_channels, 
                   int kernel_size, int stride, float *output);
extern void relu(float *data, int len);
extern void sigmoid(float *data, int len);
extern float gelu(float x);
extern void flatten_patches(const float *patches, int num_patches, int channels, float *flat);

void swiglu(float *val, float *gate, int len, float *output) {
    for (int i = 0; i < len; i++) {
        output[i] = val[i] * gelu(gate[i]);
    }
}

// Squeeze-and-Excitation block forward pass
void se_forward(float *input, int num_patches, int channels,
                const float *fc0_w, const float *fc0_b,
                const float *fc2_w, const float *fc2_b,
                float *output) {
    
    // 1. Squeeze: Global Average Pooling for EACH channel
    float pooled[16] = {0}; // Assumes max 16 channels as per current architecture
    for (int c = 0; c < channels; c++) {
        double sum = 0;
        for (int p = 0; p < num_patches; p++) {
            sum += input[p * channels + c];
        }
        pooled[c] = (float)(sum / num_patches);
    }

    // 2. Excitation: Reduction (16 -> 4)
    float fc0_out[4] = {0};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < channels; j++) {
            // Weight shape [4, 16, 1] -> [out][in]
            fc0_out[i] += pooled[j] * fc0_w[i * channels + j];
        }
        fc0_out[i] += fc0_b[i];
    }
    relu(fc0_out, 4);

    // 3. Excitation: Expansion (4 -> 16)
    float excitation[16] = {0};
    for (int i = 0; i < channels; i++) {
        for (int j = 0; j < 4; j++) {
            // Weight shape [16, 4, 1] -> [out][in]
            excitation[i] += fc0_out[j] * fc2_w[i * 4 + j];
        }
        excitation[i] += fc2_b[i];
    }
    sigmoid(excitation, channels);

    // 4. Scale: Apply weights to all patches
    for (int p = 0; p < num_patches; p++) {
        for (int c = 0; c < channels; c++) {
            output[p * channels + c] = input[p * channels + c] * excitation[c];
        }
    }
}

// Linear layer: y = xW + b
void linear(const float *input, const float *weight, const float *bias,
            int in_features, int out_features, float *output) {
    for (int o = 0; o < out_features; o++) {
        float sum = bias ? bias[o] : 0;
        for (int i = 0; i < in_features; i++) {
            sum += input[i] * weight[o * in_features + i];
        }
        output[o] = sum;
    }
}

// SE Block using 1D Conv (kernel_size=1)
void se_conv1d(const float *input, int num_patches, int channels,
               const float *w1, const float *b1,  // [inner, channels, 1] = [4, 16, 1]
               const float *w2, const float *b2,  // [channels, inner, 1] = [16, 4, 1]
               float *output) {
    int inner = channels / 4;  // 16 / 4 = 4
    
    // Global average pooling per channel
    float pooled[16] = {0};
    for (int c = 0; c < channels; c++) {
        double sum = 0;
        for (int p = 0; p < num_patches; p++) {
            sum += input[p * channels + c];
        }
        pooled[c] = (float)(sum / num_patches);
    }
    
    // First conv: channels -> inner (16 -> 4)
    // w1 shape: [inner, channels, 1] = [4, 16, 1]
    // Index: w1[inner_i * channels + channel_j]
    float hidden[4] = {0};
    for (int i = 0; i < inner; i++) {
        for (int j = 0; j < channels; j++) {
            hidden[i] += pooled[j] * w1[i * channels + j];
        }
        hidden[i] += b1[i];
    }
    relu(hidden, 4);  // ReLU
    
    // Second conv: inner -> channels (4 -> 16)
    // w2 shape: [channels, inner, 1] = [16, 4, 1]
    // Index: w2[channel_i * inner + inner_j]
    float scale[16] = {0};
    for (int i = 0; i < channels; i++) {
        for (int j = 0; j < inner; j++) {
            scale[i] += hidden[j] * w2[i * inner + j];
        }
        scale[i] += b2[i];
    }
    sigmoid(scale, 16);  // Sigmoid
    
    // Apply scale
    for (int p = 0; p < num_patches; p++) {
        for (int c = 0; c < channels; c++) {
            output[p * channels + c] = input[p * channels + c] * scale[c];
        }
    }
}

// PatchLinear forward for ONE channel
void patchlinear_channel(const float *channel_data, int seq_len,
                         const float *conv_w, const float *conv_b,
                         const float *se_w1, const float *se_b1,
                         const float *se_w2, const float *se_b2,
                         const float *head_w, const float *head_b,
                         float *output) {
    // Padding calculation (same as Python)
    int pad_len = STRIDE - ((seq_len - PATCH_LEN) % STRIDE);
    if (pad_len == STRIDE) pad_len = 0;
    int padded_len = seq_len + pad_len;
    int num_patches = (padded_len - PATCH_LEN) / STRIDE + 1;
    
    // Front replication padding (like Python's nn.ReplicationPad1d((0, pad_len)))
    float padded[400] = {0};
    for (int i = 0; i < seq_len; i++) {
        padded[i] = channel_data[i];
    }
    for (int i = seq_len; i < padded_len; i++) {
        padded[i] = channel_data[seq_len - 1];
    }
    
    // Use optimized assembly for Conv1d: 1 -> 32 channels
    // Output layout: [num_patches, 32]
    float conv_out[49 * 32] = {0};
    conv1d(padded, padded_len, conv_w, conv_b, 1, 32, 30, STRIDE, conv_out);
    
    // SwiGLU: Split into val and gate, both 16 channels
    // conv_out[p][0:16] = val, conv_out[p][16:32] = gate
    // Output layout: [num_patches, 16]
    float swiglu_out[49 * 16] = {0};
    for (int p = 0; p < num_patches; p++) {
        // Extract val and gate arrays for this patch
        float val[16];
        float gate[16];
        for (int c = 0; c < 16; c++) {
            val[c] = conv_out[p * 32 + c];
            gate[c] = conv_out[p * 32 + c + 16];
        }
        // Apply SwiGLU
        swiglu(val, gate, 16, &swiglu_out[p * 16]);
    }
    
    // SE block on SwiGLU output
    // Input/output shape: [num_patches, 16]
    float se_output[49 * 16] = {0};
    se_forward(swiglu_out, num_patches, 16, se_w1, se_b1, se_w2, se_b2, se_output);
    
    // Optimized Transpose-Flatten for head: [49, 16] -> [784]
    float flat[784] = {0};
    flatten_patches(se_output, num_patches, 16, flat);
    
    // Head linear: 784 -> 1
    *output = head_b ? head_b[0] : 0;
    for (int i = 0; i < 784; i++) {
        *output += flat[i] * head_w[i];
    }
}

// PatchLinear model forward pass for one component (trend or res)
// Processes all 4 channels together like Python does, returns prediction for channel 0
float component_forward(const float input[SEQ_LEN][NUM_FEATURES],
                        const float *conv_w, const float *conv_b,
                        const float *se_fc0_w, const float *se_fc0_b,
                        const float *se_fc2_w, const float *se_fc2_b,
                        const float *head_w, const float *head_b) {
    
    // Process each of 4 channels independently (like Python's z.reshape(B*C, 1, -1))
    float channel_preds[NUM_FEATURES] = {0};
    
    for (int c = 0; c < NUM_FEATURES; c++) {
        float channel_data[SEQ_LEN];
        for (int t = 0; t < SEQ_LEN; t++) {
            channel_data[t] = input[t][c];
        }
        
        patchlinear_channel(channel_data, SEQ_LEN,
                           conv_w, conv_b,
                           se_fc0_w, se_fc0_b,
                           se_fc2_w, se_fc2_b,
                           head_w, head_b,
                           &channel_preds[c]);
    }
    
    // Return prediction for channel 0 (Close price)
    return channel_preds[0];
}

// Full model forward pass
float model_predict(const float log_returns[SEQ_LEN][NUM_FEATURES]) {
    float normalized[SEQ_LEN][NUM_FEATURES];
    float mean[NUM_FEATURES], stdev[NUM_FEATURES];

    // Copy input
    for (int i = 0; i < SEQ_LEN; i++) {
        for (int f = 0; f < NUM_FEATURES; f++) {
            normalized[i][f] = log_returns[i][f];
        }
    }

    // RevIN normalization
    revin_normalize(normalized, mean, stdev);

    // Decomposition
    float trend[SEQ_LEN][NUM_FEATURES];
    float residual[SEQ_LEN][NUM_FEATURES];
    decompose(normalized, trend, residual);

    // Trend component
    float trend_pred = component_forward(
        trend,
        model_trend_patch_conv_conv_weight,
        model_trend_patch_conv_conv_bias,
        model_trend_patch_conv_se_fc_0_weight,
        model_trend_patch_conv_se_fc_0_bias,
        model_trend_patch_conv_se_fc_2_weight,
        model_trend_patch_conv_se_fc_2_bias,
        model_trend_head_linear_weight,
        model_trend_head_linear_bias
    );

    // Residual component
    float res_pred = component_forward(
        residual,
        model_res_patch_conv_conv_weight,
        model_res_patch_conv_conv_bias,
        model_res_patch_conv_se_fc_0_weight,
        model_res_patch_conv_se_fc_0_bias,
        model_res_patch_conv_se_fc_2_weight,
        model_res_patch_conv_se_fc_2_bias,
        model_res_head_linear_weight,
        model_res_head_linear_bias
    );

    // Final prediction (on normalized scale with affine)
    float pred_affine = trend_pred + res_pred;
    
    // RevIN denormalization for channel 0 (Close price) - match Python exactly
    const float eps = 1e-5f;
    // Step 1: Reverse affine: (x - bias) / weight
    float pred_norm = (pred_affine - revin_layer_affine_bias[0]) / (revin_layer_affine_weight[0] + eps * eps);
    // Step 2: Reverse normalization: x * stdev + mean
    float pred_denormalized = pred_norm * stdev[0] + mean[0];
    
    return pred_denormalized;
}

// Get tomorrow's date string
void get_tomorrow_date(char *buf, size_t len) {
    time_t now = time(NULL);
    time_t tomorrow = now + 24 * 3600;
    struct tm *tm = gmtime(&tomorrow);
    strftime(buf, len, "%Y-%m-%d", tm);
}


// Main prediction function
int main(int argc, char *argv[]) {
    const char *coin = (argc > 1) ? argv[1] : "BTC";
    char symbol[16];
    snprintf(symbol, sizeof(symbol), "%sUSDT", coin);

    // Step 1 & 2: Download and load model weights from GitHub
    if (load_model_from_github(coin) != 0) {
        return 1;
    }

    // Step 3: Fetch market data
    printf("Fetching data for %s...\n", symbol);

    float data[MAX_CANDLES];
    int num_candles = fetch_json_data(symbol, 367, data);
    if (num_candles <= 0) {
        fprintf(stderr, "Error: Failed to fetch JSON data\n");
        return 1;
    }

    if (num_candles < 366) {
        fprintf(stderr, "Error: Insufficient data. Got %d candles, need at least 366\n", num_candles);
        return 1;
    }

    printf("Received %d days of data\n", num_candles);

    // Compute log returns (need 366 points to get 365 returns)
    float log_returns[SEQ_LEN][NUM_FEATURES];
    compute_log_returns(data, num_candles, log_returns);

    // Run prediction
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    float pred_log_ret = model_predict(log_returns);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) * 1000.0 + 
                        (end.tv_nsec - start.tv_nsec) / 1000000.0;

    float pred_pct = (expf(pred_log_ret) - 1.0f) * 100.0f;

    // Get current and yesterday's prices
    float yesterday_close = data[num_candles - 2];
    float current_price = data[num_candles - 1];
    float actual_change = ((current_price / yesterday_close) - 1.0f) * 100.0f;

    // Get tomorrow's date
    char tomorrow_date[32];
    get_tomorrow_date(tomorrow_date, sizeof(tomorrow_date));

    // Output results
    printf("\n");
    printf("========================================\n");
    printf("Asset: %s\n", coin);
    printf("Prediction Day: %s (UTC 00:00+)\n", tomorrow_date);
    printf("----------------------------------------\n");
    printf("Model Forecast: %s (%+.4f%%)\n", (pred_log_ret > 0) ? "UP" : "DOWN", pred_pct);
    printf("Real-time (Now): %+.4f%% (Price: %.2f)\n", actual_change, current_price);
    printf("Inference Time: %.2f ms\n", time_taken);
    printf("========================================\n");

    return 0;
}

#ifndef INFERENCE_H
#define INFERENCE_H

#include <curl/curl.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

#include "../../3libs/log.h"
#include "../../3libs/sds.h"
#include "../../3libs/stb_sprintf.h"
#include "../../3libs/yyjson.h"
#include "cache.h"
#include "models.h"

struct BinaryBuffer {
  uint8_t *data;
  size_t max_size;
  size_t current_size;
};

static size_t curl_write_sds_cb(void *contents, size_t size, size_t nmemb,
                                void *userp) {
  size_t realsize = size * nmemb;
  sds *str = (sds *)userp;
  *str = sdscatlen(*str, contents, realsize);
  return realsize;
}

static size_t curl_write_bin_cb(void *contents, size_t size, size_t nmemb,
                                void *userp) {
  size_t realsize = size * nmemb;
  struct BinaryBuffer *buf = (struct BinaryBuffer *)userp;
  if (buf->current_size + realsize > buf->max_size) return 0;
  memcpy(buf->data + buf->current_size, contents, realsize);
  buf->current_size += realsize;
  return realsize;
}

static inline int fetch_binance_data(const char *symbol, Kline *out,
                                     int limit) {
  int n = 0;
  static __thread CURL *curl = NULL;
  if (!curl) {
    curl = curl_easy_init();
  } else {
    curl_easy_reset(curl);
  }
  if (!curl) return 0;

  char url[256];
  stbsp_snprintf(
      url, sizeof(url),
      "https://api.binance.com/api/v3/klines?symbol=%s&interval=1d&limit=%d",
      symbol, limit);

  sds response = sdsempty();
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_sds_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "A35M-Engine/1.0");
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK || sdslen(response) == 0) {
    log_error("Binance API fetch failed: %s", curl_easy_strerror(res));
    sdsfree(response);
    return 0;
  }

  yyjson_doc *doc = yyjson_read(response, sdslen(response), 0);
  if (doc) {
    yyjson_val *root = yyjson_doc_get_root(doc);
    if (yyjson_is_arr(root)) {
      size_t idx, max;
      yyjson_val *item;
      yyjson_arr_foreach(root, idx, max, item) {
        if (n >= limit) break;
        if (!yyjson_is_arr(item)) continue;
        yyjson_val *ts = yyjson_arr_get(item, 0);
        yyjson_val *cl = yyjson_arr_get(item, 4);
        if (ts && cl && yyjson_is_str(cl)) {
          const char *cl_str = yyjson_get_str(cl);
          if (cl_str && cl_str[0]) {
            out[n].timestamp = (long long)yyjson_get_num(ts);
            out[n].close = atof(cl_str);
            if (out[n].close > 0) n++;
          }
        }
      }
    }
    yyjson_doc_free(doc);
  }
  sdsfree(response);
  return n;
}

static inline int download_model_from_github(const char *coin, uint8_t *out,
                                             int sz) {
  static __thread CURL *curl = NULL;
  if (!curl) {
    curl = curl_easy_init();
  } else {
    curl_easy_reset(curl);
  }
  if (!curl) return 0;

  char url[256];
  stbsp_snprintf(
      url, sizeof(url),
      "https://raw.githubusercontent.com/KhanhAI-VN/Test/main/%s.bin", coin);

  struct BinaryBuffer buf = {out, (size_t)sz, 0};

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_bin_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "A35M-Engine/1.0");
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

  CURLcode res = curl_easy_perform(curl);

  long http_code = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

  if (res != CURLE_OK || http_code != 200) {
    log_error("Failed to download model for %s (curl: %s, http: %ld)", coin,
              curl_easy_strerror(res), http_code);
    return 0;
  }

  return (int)buf.current_size;
}

static inline PredictionResult run_prediction(const char *coin) {
  const char *cname = coin ? coin : "BTC";
  for (const char *p = cname; *p; p++) {
    if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
          (*p >= '0' && *p <= '9'))) {
      log_warn("Invalid symbol rejected: %s", cname);
      return (PredictionResult){
          .success = 0, .coin = "", .error_msg = "Invalid symbol"};
    }
  }
  char symbol[32];
  stbsp_snprintf(symbol, sizeof(symbol), "%sUSDT", cname);

  CoinCache *cache = get_coin_cache(cname);
  pthread_mutex_lock(&cache->coin_mutex);

  if (should_update_cache(cache)) {
    pthread_mutex_unlock(&cache->coin_mutex);

    uint8_t *l_model_buf = malloc(18 * 1024);
    Kline *l_kline_buf = malloc((SEQ_LEN + 2) * sizeof(Kline));
    if (!l_model_buf || !l_kline_buf) {
      free(l_model_buf);
      free(l_kline_buf);
      log_error("Memory allocation failed for coin %s", cname);
      return (PredictionResult){
          .success = 0, .coin = "", .error_msg = "Memory error"};
    }

    int m_len = download_model_from_github(cname, l_model_buf, 18 * 1024);
    int f_count = fetch_binance_data(symbol, l_kline_buf, SEQ_LEN + 2);

    cache = get_coin_cache(cname);
    pthread_mutex_lock(&cache->coin_mutex);
    if (should_update_cache(cache)) {
      uint8_t pool_idx = (uint8_t)(cache - caches);
      if (m_len > 0) cache->model = load_model(pool_idx, l_model_buf, m_len);

      if (!cache->model || f_count < SEQ_LEN + 2) {
        log_error(
            "%s: %s (m_len=%d f_count=%d)", cname,
            !cache->model ? "model load failed" : "insufficient kline data",
            m_len, f_count);
        PredictionResult res = {0};
        memcpy(res.coin, cache->coin, sizeof(res.coin));
        stbsp_snprintf(res.error_msg, sizeof(res.error_msg), "%s",
                       !cache->model ? "Model error" : "Data error");
        pthread_mutex_unlock(&cache->coin_mutex);
        free(l_model_buf);
        free(l_kline_buf);
        return res;
      }

      cache->kline_count = f_count;
      memcpy(cache->klines, l_kline_buf, f_count * sizeof(Kline));
      for (int i = 0; i < SEQ_LEN; i++) {
        float c1 = (float)cache->klines[i].close,
              c2 = (float)cache->klines[i + 1].close;
        cache->input[i] = (c1 > 1e-9f && c2 > 1e-9f) ? logf(c2 / c1) : 0;
      }

      update_cache_day(cache);

      float local_input[SEQ_LEN];
      memcpy(local_input, cache->input, sizeof(local_input));
      Model *m = cache->model;

      struct timeval start, end;
      gettimeofday(&start, NULL);
      float p_val = predict(m, local_input);
      gettimeofday(&end, NULL);

      cache->pred_log_diff = p_val;
      cache->last_res.success = !isnan(p_val) && !isinf(p_val);
      cache->last_res.inference_ms = (end.tv_sec - start.tv_sec) * 1000.0 +
                                     (end.tv_usec - start.tv_usec) / 1000.0;
      cache->last_res.pred_change_pct =
          cache->last_res.success ? (expf(p_val) - 1.0f) * 100.0f : 0;

      time_t pred_ts =
          (cache->klines[cache->kline_count - 1].timestamp) / 1000 + 86400;
      struct tm t_pred;
      gmtime_r(&pred_ts, &t_pred);
      strftime(cache->last_res.date, sizeof(cache->last_res.date), "%Y-%m-%d",
               &t_pred);
    }
    free(l_model_buf);
    free(l_kline_buf);
  }

  pthread_mutex_unlock(&cache->coin_mutex);
  Kline latest[2] = {0};
  int n = fetch_binance_data(symbol, latest, 2);
  pthread_mutex_lock(&cache->coin_mutex);

  if (n == 2) {
    cache->last_res.last_price = (float)latest[1].close;
    float yesterday_close = (float)latest[0].close;
    cache->last_res.change_pct =
        (yesterday_close > 1e-9)
            ? ((cache->last_res.last_price - yesterday_close) /
               yesterday_close) *
                  100.0f
            : 0;
    cache->last_res.pred_price =
        cache->last_res.last_price * expf(cache->pred_log_diff);
    cache->last_res.trend =
        (cache->last_res.pred_price > cache->last_res.last_price) ? 1 : 0;
  }

  PredictionResult res = cache->last_res;
  memcpy(res.coin, cache->coin, sizeof(res.coin));
  pthread_mutex_unlock(&cache->coin_mutex);
  return res;
}

static inline void trigger_github_retrain(const char *token) {
  static __thread CURL *curl = NULL;
  if (!curl) {
    curl = curl_easy_init();
  } else {
    curl_easy_reset(curl);
  }
  if (!curl) return;

  const char *url =
      "https://api.github.com/repos/KhanhAI-VN/Test/actions/"
      "workflows/retrain.yml/dispatches";
  const char *body = "{\"ref\":\"main\"}";

  struct curl_slist *headers = NULL;
  char auth_header[256];
  stbsp_snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s",
                 token);

  headers = curl_slist_append(headers, "Accept: application/vnd.github+json");
  headers = curl_slist_append(headers, auth_header);
  headers = curl_slist_append(headers, "X-GitHub-Api-Version: 2022-11-28");
  headers = curl_slist_append(headers, "User-Agent: Luckfox-Pico");
  headers = curl_slist_append(headers, "Content-Type: application/json");

  sds response = sdsempty();

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_sds_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    log_error("trigger_github_retrain failed: %s", curl_easy_strerror(res));
  } else {
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code >= 200 && http_code < 300) {
      log_info("Retrain triggered successfully (HTTP %ld)", http_code);
    } else {
      log_error("Retrain failed with HTTP %ld: %s", http_code, response);
    }
  }

  curl_slist_free_all(headers);
  sdsfree(response);
}

#endif

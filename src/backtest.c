#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "include/inference.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "../3libs/stb_sprintf.h"

typedef struct {
  double open, high, low, close, volume;
  long long timestamp;
} OHLC;

#define N_COINS 40
#define START_CAPITAL 20.0f
#define TRADE_MARGIN 1.0f
#define LEVERAGE 5.0f
#define FEE_PCT 0.001f
#define SL_PCT 0.10f

int fetch_ohlc_multi(const char *symbol, OHLC *out, int limit,
                     const char *interval) {
  int total_fetched = 0;
  long long last_ts = 0;

  static __thread CURL *curl = NULL;
  if (!curl) {
    curl = curl_easy_init();
  }
  if (!curl) return 0;

  while (total_fetched < limit) {
    int page_limit =
        (limit - total_fetched > 1000) ? 1000 : (limit - total_fetched);
    char url[512];

    if (last_ts == 0) {
      stbsp_snprintf(url, sizeof(url),
                     "https://api.binance.com/api/v3/"
                     "klines?symbol=%s&interval=%s&limit=%d",
                     symbol, interval, page_limit);
    } else {
      stbsp_snprintf(url, sizeof(url),
                     "https://api.binance.com/api/v3/"
                     "klines?symbol=%s&interval=%s&limit=%d&endTime=%lld",
                     symbol, interval, page_limit, last_ts - 1);
    }

    sds response = sdsempty();
    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_sds_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "A35M-Engine/1.0");
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK || sdslen(response) == 0) {
      log_error("Backtest fetch failed: %s", curl_easy_strerror(res));
      sdsfree(response);
      break;
    }

    yyjson_doc *doc = yyjson_read(response, sdslen(response), 0);
    sdsfree(response);

    if (!doc) break;
    yyjson_val *root = yyjson_doc_get_root(doc);
    if (!yyjson_is_arr(root)) {
      yyjson_doc_free(doc);
      break;
    }

    int n_page = 0;
    OHLC page_data[1000];

    size_t idx, max;
    yyjson_val *item;
    yyjson_arr_foreach(root, idx, max, item) {
      if (n_page >= page_limit) break;
      if (!yyjson_is_arr(item)) continue;

      yyjson_val *ts = yyjson_arr_get(item, 0);
      yyjson_val *so = yyjson_arr_get(item, 1);
      yyjson_val *sh = yyjson_arr_get(item, 2);
      yyjson_val *sl = yyjson_arr_get(item, 3);
      yyjson_val *sc = yyjson_arr_get(item, 4);
      yyjson_val *sv = yyjson_arr_get(item, 5);

      if (ts && so && sh && sl && sc && sv && yyjson_is_str(sc)) {
        page_data[n_page].timestamp = (long long)yyjson_get_num(ts);
        page_data[n_page].open = atof(yyjson_get_str(so));
        page_data[n_page].high = atof(yyjson_get_str(sh));
        page_data[n_page].low = atof(yyjson_get_str(sl));
        page_data[n_page].close = atof(yyjson_get_str(sc));
        page_data[n_page].volume = atof(yyjson_get_str(sv));
        n_page++;
      }
    }
    yyjson_doc_free(doc);

    if (n_page == 0) break;

    memmove(out + n_page, out, total_fetched * sizeof(OHLC));
    memcpy(out, page_data, n_page * sizeof(OHLC));

    total_fetched += n_page;
    last_ts = page_data[0].timestamp;
    if (n_page < page_limit) break;
  }
  return total_fetched;
}

void resample_to_daily(OHLC *hourly, int n_hours, OHLC *daily_out, int *n_days,
                       int *offset_out) {
  int start_idx = 0;

  while (start_idx < n_hours && (hourly[start_idx].timestamp % 86400000) != 0) {
    start_idx++;
  }
  if (offset_out) *offset_out = start_idx;

  int d = 0;
  for (int i = start_idx; i <= n_hours - 24; i += 24) {
    daily_out[d].timestamp = hourly[i].timestamp;
    daily_out[d].open = hourly[i].open;
    daily_out[d].close = hourly[i + 23].close;
    double high = hourly[i].high, low = hourly[i].low, vol = 0;
    for (int j = 0; j < 24; j++) {
      if (hourly[i + j].high > high) high = hourly[i + j].high;
      if (hourly[i + j].low < low) low = hourly[i + j].low;
      vol += hourly[i + j].volume;
    }
    daily_out[d].high = high;
    daily_out[d].low = low;
    daily_out[d].volume = vol;
    d++;
  }
  *n_days = d;
}

int main() {
  const char *coins[N_COINS] = {"ETH", "BTC", "BNB", "LTC", "ADA", "XLM", "XRP", "TRX", "LINK",
                                "FET", "DASH", "ZEC", "ATOM", "DUSK", "DOGE", "HBAR", "CHZ", "STORJ", "JST",
                                "DOT", "NMR", "SOL", "RUNE", "AVAX", "UNI",
                                "BCH", "NEAR", "AAVE", "FIL", "SHIB", "APE", "INJ", "ETC", "APT", "PHB",
                                "TON", "SUI", "PEPE", "WLD", "ONDO"};
  static OHLC hourly_data[N_COINS][14000];
  static OHLC daily_data[N_COINS][600];
  int offsets[N_COINS] = {0};
  uint8_t model_bufs[N_COINS][32768];
  Model *models[N_COINS];

  printf("--- Shrimp Precise Backtest (40 Coins - 1H Path) ---\n");
  printf("Leverage: %.0fx | Shared Capital: $%.2f | Margin: $%.2f\n", LEVERAGE,
         START_CAPITAL, TRADE_MARGIN);

  int min_days = 999;
  for (int c = 0; c < N_COINS; c++) {
    char symbol[32];
    sprintf(symbol, "%sUSDT", coins[c]);
    int nh = fetch_ohlc_multi(symbol, hourly_data[c], 13200, "1h");
    int nd = 0;
    resample_to_daily(hourly_data[c], nh, daily_data[c], &nd, &offsets[c]);

    if (nd < SEQ_LEN + 8) {
      printf("Error: Not enough data for %s (%d days)\n", coins[c], nd);
      return 1;
    }
    if (nd < min_days) min_days = nd;

    int m_len = download_model_from_github(coins[c], model_bufs[c], 32768);
    models[c] = load_model(c, model_bufs[c], m_len);
    if (!models[c]) {
      printf("Error: Model load failed for %s\n", coins[c]);
      return 1;
    }
  }

  float capital = START_CAPITAL;
  float prev_capital = START_CAPITAL;
  float peak_capital = START_CAPITAL;
  float max_dd = 0.0f;
  int pos[N_COINS] = {0};
  float entry[N_COINS] = {0}, entry_notional[N_COINS] = {0},
        coin_pnl[N_COINS] = {0};
  int coin_trades[N_COINS] = {0}, coin_wins[N_COINS] = {0}, coin_sl[N_COINS] = {0};

  int total_days = min_days;
  int start_day = total_days - 365;

  printf("\n  DATE         BALANCE   PnL%%\n");
  printf("  -----------------------------------\n");

  for (int d = start_day; d < total_days - 1; d++) {
    int daily_trends[N_COINS] = {0};
    float preds[N_COINS] = {0};

    for (int c = 0; c < N_COINS; c++) {
      float input[SEQ_LEN];
      float sum_vol = 0;
      for (int j = 0; j < SEQ_LEN; j++) {
        int idx = d - 1 - SEQ_LEN + j;
        float c1 = (float)daily_data[c][idx].close;
        float c2 = (float)daily_data[c][idx + 1].close;
        input[j] = (c1 > 0 && c2 > 0) ? logf(c2 / c1) : 0;
        sum_vol += fabsf(input[j]);
      }
      preds[c] = predict(models[c], input);
      float avg_vol = sum_vol / SEQ_LEN;
      daily_trends[c] = (preds[c] >= avg_vol * 1.0f) ? 1 : 0;
    }

    int n_down = 0;
    for (int c = 0; c < N_COINS; c++) {
      if (daily_trends[c] == 0) n_down++;
    }
    if (n_down >= (int)(N_COINS * 0.70f)) {
      for (int c = 0; c < N_COINS; c++) {
        daily_trends[c] = 0;
      }
    }

    // Sắp xếp coin UP theo pred giảm dần
    int up_coins[N_COINS];
    int n_up = 0;
    for (int c = 0; c < N_COINS; c++) {
      if (daily_trends[c] == 1) up_coins[n_up++] = c;
    }
    // Insertion sort by pred descending
    for (int i = 1; i < n_up; i++) {
      int key = up_coins[i];
      float key_pred = preds[key];
      int j = i - 1;
      while (j >= 0 && preds[up_coins[j]] < key_pred) {
        up_coins[j + 1] = up_coins[j];
        j--;
      }
      up_coins[j + 1] = key;
    }

    // Đóng lệnh: coin không còn tín hiệu UP
    for (int c = 0; c < N_COINS; c++) {
      if (pos[c] == 1 && daily_trends[c] == 0) {
        int h_start = offsets[c] + d * 24;
        float exit_price = hourly_data[c][h_start].open;
        float pnl = (exit_price - entry[c]) / entry[c] * entry_notional[c];
        float fee = entry_notional[c] * FEE_PCT * 2.0f;
        capital += (pnl - fee);
        coin_pnl[c] += (pnl - fee);
        coin_trades[c]++;
        if (pnl > fee) coin_wins[c]++;
        pos[c] = 0;
      }
    }

    // Tính coin cần mở mới
    int new_open[N_COINS] = {0};
    int n_need_open = 0;
    for (int i = 0; i < n_up; i++) {
      int c = up_coins[i];
      if (pos[c] == 0) {
        new_open[n_need_open++] = c;
      }
    }

    // Chia hết toàn bộ vốn, chỉ giới hạn 1% thanh khoản
    if (n_need_open > 0) {
      float notional_alloc[N_COINS] = {0};
      int capped[N_COINS] = {0};
      float remaining_capital = capital;
      int remaining_coins = n_need_open;

      // Lặp phân bổ: coin bị cap volume → dư ra chia cho coin còn lại
      for (int pass = 0; pass < n_need_open && remaining_coins > 0; pass++) {
        float margin_each = remaining_capital / remaining_coins;
        if (margin_each > capital * 0.10f) {
          margin_each = capital * 0.10f;
        }
        float notional_each = margin_each * LEVERAGE;
        int any_capped = 0;

        for (int i = 0; i < n_need_open; i++) {
          if (capped[i]) continue;
          int c = new_open[i];
          float vol_cap = daily_data[c][d - 1].volume *
                          daily_data[c][d - 1].close * 0.01f;
          if (notional_each > vol_cap) {
            notional_alloc[i] = vol_cap;
            capped[i] = 1;
            remaining_capital -= vol_cap / LEVERAGE;
            remaining_coins--;
            any_capped = 1;
          }
        }

        if (!any_capped) {
          // Không coin nào bị cap → chia đều phần còn lại
          for (int i = 0; i < n_need_open; i++) {
            if (!capped[i]) {
              notional_alloc[i] = margin_each * LEVERAGE;
            }
          }
          break;
        }
      }

      // Mở lệnh
      for (int i = 0; i < n_need_open; i++) {
        if (notional_alloc[i] <= 0) continue;
        int c = new_open[i];
        int h_start = offsets[c] + d * 24;
        pos[c] = 1;
        entry[c] = hourly_data[c][h_start].open;
        entry_notional[c] = notional_alloc[i];
      }
    }

    // Check SL + Take-profit mỗi giờ
    #define TP_NET_PCT 4.5f
    #define TP_FEE_PER_COIN 0.2f
    for (int h = 0; h < 24; h++) {
      // 1. Check SL từng coin
      for (int c = 0; c < N_COINS; c++) {
        if (pos[c] == 1) {
          int h_idx = offsets[c] + d * 24 + h;
          OHLC hour = hourly_data[c][h_idx];
          if (hour.low <= entry[c] * (1.0f - SL_PCT)) {
            float loss = entry_notional[c] * SL_PCT;
            float fee = entry_notional[c] * FEE_PCT * 2.0f;
            capital -= (loss + fee);
            coin_pnl[c] -= (loss + fee);
            coin_sl[c]++;
            coin_trades[c]++;
            pos[c] = 0;
          }
        }
      }

      // 2. Tính Total UP và Total Fee (giống web.html)
      float total_up = 0;
      int n_open = 0;
      for (int c = 0; c < N_COINS; c++) {
        if (pos[c] == 1) {
          int h_idx = offsets[c] + d * 24 + h;
          float cur_price = hourly_data[c][h_idx].close;
          float pct_change = (cur_price - entry[c]) / entry[c] * 100.0f;
          total_up += pct_change;
          n_open++;
        }
      }
      float total_fee = n_open * TP_FEE_PER_COIN;
      float net = total_up - total_fee;

      // 3. Nếu net >= 4.5% → đóng toàn bộ lệnh chốt lời
      if (n_open > 0 && net >= TP_NET_PCT) {
        for (int c = 0; c < N_COINS; c++) {
          if (pos[c] == 1) {
            int h_idx = offsets[c] + d * 24 + h;
            float exit_price = hourly_data[c][h_idx].close;
            float pnl = (exit_price - entry[c]) / entry[c] * entry_notional[c];
            float fee = entry_notional[c] * FEE_PCT * 2.0f;
            capital += (pnl - fee);
            coin_pnl[c] += (pnl - fee);
            coin_trades[c]++;
            if (pnl > fee) coin_wins[c]++;
            pos[c] = 0;
          }
        }
        break; // Đã đóng hết, không cần check giờ tiếp
      }
    }

    float daily_portfolio_value = capital;
    for (int c = 0; c < N_COINS; c++) {
      if (pos[c] == 1) {
        float current_price = daily_data[c][d].close;
        float pnl = (current_price - entry[c]) / entry[c] * entry_notional[c];
        float fee = entry_notional[c] * FEE_PCT * 2.0f;
        daily_portfolio_value += (pnl - fee);
      }
    }

    // In PnL% so với ngày trước
    float day_pnl_pct = (prev_capital > 0) ? (daily_portfolio_value - prev_capital) / prev_capital * 100.0f : 0;
    time_t ts = (time_t)(daily_data[0][d].timestamp / 1000);
    struct tm *tm = gmtime(&ts);
    printf("  %04d-%02d-%02d  $%7.2f  %+6.2f%%\n",
           tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
           daily_portfolio_value, day_pnl_pct);
    prev_capital = daily_portfolio_value;

    if (daily_portfolio_value > peak_capital) {
      peak_capital = daily_portfolio_value;
    }

    float dd = (peak_capital > 0) ? (peak_capital - daily_portfolio_value) /
                                        peak_capital * 100.0f
                                  : 0;
    if (dd > max_dd) max_dd = dd;

    if (capital <= 0) {
      printf("BANKRUPT at day %d\n", d);
      break;
    }
  }

  for (int c = 0; c < N_COINS; c++) {
    if (pos[c] == 1 && capital > 0) {
      float pnl = (daily_data[c][total_days - 1].close - entry[c]) / entry[c] *
                  entry_notional[c];
      float fee = entry_notional[c] * FEE_PCT * 2.0f;
      capital += (pnl - fee);
      coin_pnl[c] += (pnl - fee);
      coin_trades[c]++;
      if (pnl > fee) coin_wins[c]++;
    }
  }

  float total_pnl_pct = (START_CAPITAL > 0) ? (capital - START_CAPITAL) / START_CAPITAL * 100.0f : 0;
  printf("-------------------------------------------\n");
  printf("  Final: $%.2f | PnL: %+.2f%% | MDD: %.2f%%\n", capital, total_pnl_pct, max_dd);
  printf("-------------------------------------------\n");

  return 0;
}
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

#define N_COINS 10
#define START_CAPITAL 20.0f
#define TRADE_MARGIN 2.5f
#define LEVERAGE 5.0f
#define FEE_PCT 0.001f
#define SL_PCT 0.1f
#define MIN_CONF_UP 0.0198f

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
  const char *coins[N_COINS] = {"BTC", "ETH", "SOL", "SHIB", "ADA", "XRP", "DOGE", "LINK", "BNB", "AVAX"};
  static OHLC hourly_data[N_COINS][14000];
  static OHLC daily_data[N_COINS][600];
  int offsets[N_COINS] = {0};
  uint8_t model_bufs[N_COINS][32768];
  Model *models[N_COINS];

  printf("--- Shrimp Precise Backtest (10 Coins - 1H Path) ---\n");
  printf("Leverage: %.0fx | Shared Capital: $%.2f | Margin: $%.2f\n", LEVERAGE,
         START_CAPITAL, TRADE_MARGIN);

  int min_days = 999;
  for (int c = 0; c < N_COINS; c++) {
    char symbol[32];
    sprintf(symbol, "%sUSDT", coins[c]);
    int nh = fetch_ohlc_multi(symbol, hourly_data[c], 13200, "1h");
    int nd = 0;
    resample_to_daily(hourly_data[c], nh, daily_data[c], &nd, &offsets[c]);

    if (nd < SEQ_LEN + 181) {
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
  float peak_capital = START_CAPITAL;
  float max_dd = 0.0f;
  int pos[N_COINS] = {0};
  float entry[N_COINS] = {0}, entry_notional[N_COINS] = {0},
        coin_pnl[N_COINS] = {0};
  int coin_trades[N_COINS] = {0}, coin_wins[N_COINS] = {0}, coin_sl[N_COINS] = {0};

  int total_days = min_days;
  int start_day = total_days - 180;

  for (int d = start_day; d < total_days - 1; d++) {
    int daily_trends[N_COINS] = {0};
    float preds[N_COINS] = {0};

    for (int c = 0; c < N_COINS; c++) {
      float input[SEQ_LEN];
      for (int j = 0; j < SEQ_LEN; j++) {
        int idx = d - 1 - SEQ_LEN + j;
        float c1 = (float)daily_data[c][idx].close;
        float c2 = (float)daily_data[c][idx + 1].close;
        input[j] = (c1 > 0 && c2 > 0) ? logf(c2 / c1) : 0;
      }
      preds[c] = predict(models[c], input);
      daily_trends[c] = (preds[c] >= MIN_CONF_UP) ? 1 : 0;
    }

    int n_down = 0;
    for (int c = 0; c < N_COINS; c++) {
      if (daily_trends[c] == 0) n_down++;
    }
    if (n_down >= (int)(N_COINS * 0.9f)) {
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

    // Tính vốn khả dụng và số coin cần mở mới
    int new_open[N_COINS] = {0};
    int n_need_open = 0;
    for (int i = 0; i < n_up; i++) {
      int c = up_coins[i];
      if (pos[c] == 0) {
        new_open[n_need_open++] = c;
      }
    }

    // Chia vốn đều: giảm số coin cho đến khi mỗi coin >= $2 margin
    #define MIN_MARGIN 2.0f
    int n_alloc = n_need_open;
    while (n_alloc > 0 && (capital / n_alloc) < MIN_MARGIN) {
      n_alloc--;
    }

    // Mở lệnh từ coin có pred cao nhất xuống, chỉ n_alloc coin
    for (int i = 0; i < n_alloc; i++) {
      int c = new_open[i];
      int h_start = offsets[c] + d * 24;
      float current_margin = capital / n_alloc;
      float max_cap_per_coin = 0.20f;
      if (current_margin > capital * max_cap_per_coin)
        current_margin = capital * max_cap_per_coin;
      float current_notional = current_margin * LEVERAGE;

      float prev_day_usd_vol =
          daily_data[c][d - 1].volume * daily_data[c][d - 1].close;
      if (current_notional > prev_day_usd_vol * 0.01f) {
        current_notional = prev_day_usd_vol * 0.01f;
      }

      pos[c] = 1;
      entry[c] = hourly_data[c][h_start].open;
      entry_notional[c] = current_notional;
    }

    for (int c = 0; c < N_COINS; c++) {
      if (pos[c] == 1) {
        int h_start = offsets[c] + d * 24;
        for (int h = 0; h < 24; h++) {
          OHLC hour = hourly_data[c][h_start + h];
          if (hour.low <= entry[c] * (1.0f - SL_PCT)) {
            float loss = entry_notional[c] * SL_PCT;
            float fee = entry_notional[c] * FEE_PCT * 2.0f;
            capital -= (loss + fee);
            coin_pnl[c] -= (loss + fee);
            coin_sl[c]++;
            coin_trades[c]++;
            pos[c] = 0;
            break;
          }
        }
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

  printf("\nDetailed Performance per Coin (1H Path):\n");
  printf("-----------------------------------------------------------\n");
  printf("  COIN   TRADES  WINS  SL   WR(%%)    PnL Estim.\n");
  printf("-----------------------------------------------------------\n");
  int total_t = 0, total_w = 0, total_s = 0;
  for (int c = 0; c < N_COINS; c++) {
    float wr =
        coin_trades[c] > 0 ? (float)coin_wins[c] / coin_trades[c] * 100 : 0;
    printf("  %-5s  %4d  %4d  %2d   %5.1f%%   $%+6.2f\n", coins[c], coin_trades[c],
           coin_wins[c], coin_sl[c], wr, coin_pnl[c]);
    total_t += coin_trades[c];
    total_w += coin_wins[c];
    total_s += coin_sl[c];
  }
  printf("-----------------------------------------------------------\n");
  printf("  TOTAL  %4d  %4d  %2d   %5.1f%%   Bal: $%.2f  MDD: %.2f%%\n", total_t,
         total_w, total_s, total_t > 0 ? (float)total_w / total_t * 100 : 0, capital,
         max_dd);
  printf("-----------------------------------------------------------\n");

  return 0;
}
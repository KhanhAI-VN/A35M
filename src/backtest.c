#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include "include/inference.h"

// Custom struct for backtesting
typedef struct { 
    double open, high, low, close, volume; 
    long long timestamp; 
} OHLC;

#define N_COINS 5
#define START_CAPITAL 20.0f
#define TRADE_MARGIN 2.5f
#define LEVERAGE 5.0f
#define FEE_PCT 0.001f

// Fetch multiple pages of OHLC data to overcome the 1000 limit
int fetch_ohlc_multi(const char *symbol, OHLC *out, int limit, const char *interval) {
    int total_fetched = 0;
    long long last_ts = 0;

    while (total_fetched < limit) {
        SSLConnection c = create_ssl_connection(BINANCE_HOST);
        if (!c.ssl) break;

        int page_limit = (limit - total_fetched > 1000) ? 1000 : (limit - total_fetched);
        char req[512], buf[16384], *p, *start, *end;
        
        if (last_ts == 0) {
            sprintf(req, "GET /api/v3/klines?symbol=%s&interval=%s&limit=%d HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", 
                    symbol, interval, page_limit, BINANCE_HOST);
        } else {
            sprintf(req, "GET /api/v3/klines?symbol=%s&interval=%s&limit=%d&endTime=%lld HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", 
                    symbol, interval, page_limit, last_ts - 1, BINANCE_HOST);
        }

        SSL_write(c.ssl, req, strlen(req));
        int n_page = 0, len, pos = 0, head = 0;
        OHLC page_data[1000];

        while (n_page < page_limit && (len = SSL_read(c.ssl, buf + pos, sizeof(buf) - pos - 1)) > 0) {
            buf[pos += len] = 0; p = buf;
            if (!head) {
                if (!(p = strstr(buf, "\r\n\r\n"))) {
                    // [FIX BUG] Prevent buffer overflow if headers are too large
                    if (pos > (int)sizeof(buf) - 512) { memmove(buf, buf + pos - 4, 4); pos = 4; }
                    continue;
                }
                p += 4; head = 1;
            }
            while (n_page < page_limit && (start = strstr(p, "["))) {
                if (start[1] == '[') { p = start + 1; continue; }
                if (!(end = strstr(start, "]"))) break;
                
                char so[32], sh[32], sl[32], sc[32], sv[32];
                if (sscanf(start + 1, " %lld , \"%[^\"]\" , \"%[^\"]\" , \"%[^\"]\" , \"%[^\"]\" , \"%[^\"]\"", 
                           &page_data[n_page].timestamp, so, sh, sl, sc, sv) >= 6) {
                    page_data[n_page].open = atof(so); 
                    page_data[n_page].high = atof(sh); 
                    page_data[n_page].low = atof(sl); 
                    page_data[n_page].close = atof(sc); 
                    page_data[n_page++].volume = atof(sv);
                }
                p = end + 1;
            }
            pos = buf + pos - p; memmove(buf, p, pos);
        }
        cleanup_ssl_connection(c);

        if (n_page == 0) break;

        memmove(out + n_page, out, total_fetched * sizeof(OHLC));
        memcpy(out, page_data, n_page * sizeof(OHLC));
        
        total_fetched += n_page;
        last_ts = page_data[0].timestamp; 
        if (n_page < page_limit) break; 
    }
    return total_fetched;
}

// [FIX BUG] Added offset_out to enforce exact 00:00 UTC alignment
void resample_to_daily(OHLC *hourly, int n_hours, OHLC *daily_out, int *n_days, int *offset_out) {
    int start_idx = 0;
    // Find the very first 00:00 UTC hourly candle
    while (start_idx < n_hours && (hourly[start_idx].timestamp % 86400000) != 0) {
        start_idx++;
    }
    if (offset_out) *offset_out = start_idx;

    int d = 0;
    for (int i = start_idx; i <= n_hours - 24; i += 24) {
        daily_out[d].timestamp = hourly[i].timestamp;
        daily_out[d].open = hourly[i].open;
        daily_out[d].close = hourly[i+23].close;
        double high = hourly[i].high, low = hourly[i].low, vol = 0;
        for (int j = 0; j < 24; j++) {
            if (hourly[i+j].high > high) high = hourly[i+j].high;
            if (hourly[i+j].low < low) low = hourly[i+j].low;
            vol += hourly[i+j].volume;
        }
        daily_out[d].high = high;
        daily_out[d].low = low;
        daily_out[d].volume = vol;
        d++;
    }
    *n_days = d;
}

int main() {
    const char *coins[N_COINS] = {"BTC", "ETH", "SOL", "SHIB", "ADA"};
    static OHLC hourly_data[N_COINS][14000];
    static OHLC daily_data[N_COINS][600];
    int offsets[N_COINS] = {0}; // To store UTC phase sync offsets
    uint8_t model_bufs[N_COINS][32768];
    Model *models[N_COINS];
    
    printf("--- Shrimp Precise Backtest (5 Coins - 1H Path) ---\n");
    printf("Leverage: %.0fx | Shared Capital: $%.2f | Margin: $%.2f\n", LEVERAGE, START_CAPITAL, TRADE_MARGIN);
    
    int min_days = 999;
    for (int c = 0; c < N_COINS; c++) {
        char symbol[32]; sprintf(symbol, "%sUSDT", coins[c]);
        int nh = fetch_ohlc_multi(symbol, hourly_data[c], 13200, "1h");
        int nd = 0;
        resample_to_daily(hourly_data[c], nh, daily_data[c], &nd, &offsets[c]);
        
        // [FIX BUG] nd MUST be at least SEQ_LEN + 181 to prevent `idx = -1` Out-Of-Bounds error.
        if (nd < SEQ_LEN + 181) { printf("Error: Not enough data for %s (%d days)\n", coins[c], nd); return 1; }
        if (nd < min_days) min_days = nd;
        
        int m_len = download_model_from_github(coins[c], model_bufs[c], 32768);
        model_set_pool(c);
        models[c] = load_model(model_bufs[c], m_len);
        if (!models[c]) { printf("Error: Model load failed for %s\n", coins[c]); return 1; }
    }
    
    float capital = START_CAPITAL;
    int pos[N_COINS] = {0}; 
    float entry[N_COINS] = {0}, entry_notional[N_COINS] = {0}, coin_pnl[N_COINS] = {0};
    int coin_trades[N_COINS] = {0}, coin_wins[N_COINS] = {0};

    int total_days = min_days;
    int start_day = total_days - 180;

    for (int d = start_day; d < total_days - 1; d++) {
        int daily_trends[N_COINS] = {0};

        // -------------------------------------------------------------
        // PASS 1: Generate Signals & Process 00:00 Exits
        // -------------------------------------------------------------
        for (int c = 0; c < N_COINS; c++) {
            float input[SEQ_LEN];
            for (int j = 0; j < SEQ_LEN; j++) {
                int idx = d - 1 - SEQ_LEN + j; 
                float c1 = (float)daily_data[c][idx].close;
                float c2 = (float)daily_data[c][idx+1].close;
                input[j] = (c1 > 0 && c2 > 0) ? logf(c2 / c1) : 0;
            }
            float pred = predict(models[c], input);
            daily_trends[c] = (pred > 0) ? 1 : 0; 

            int h_start = offsets[c] + d * 24;
            
            // Exit at 00:00 open if trend reverses
            if (pos[c] == 1 && daily_trends[c] == 0) {
                float exit_price = hourly_data[c][h_start].open;
                float pnl = (exit_price - entry[c]) / entry[c] * entry_notional[c];
                float fee = entry_notional[c] * FEE_PCT * 2.0f;
                capital += (pnl - fee); coin_pnl[c] += (pnl - fee); 
                coin_trades[c]++; if (pnl > fee) coin_wins[c]++;
                pos[c] = 0;
            }
        }

        // -------------------------------------------------------------
        // PASS 2: Process 00:00 Entries (Using isolated 00:00 Capital)
        // -------------------------------------------------------------
        for (int c = 0; c < N_COINS; c++) {
            int trend = daily_trends[c];
            int h_start = offsets[c] + d * 24;

            if (pos[c] == 0 && trend == 1) {
                float current_margin = capital / N_COINS;
                float current_notional = current_margin * LEVERAGE;
                
                // Volume filter protection
                float prev_day_usd_vol = daily_data[c][d-1].volume * daily_data[c][d-1].close;
                if (current_notional > prev_day_usd_vol * 0.01f) {
                    current_notional = prev_day_usd_vol * 0.01f;
                }
                
                pos[c] = 1; 
                entry[c] = hourly_data[c][h_start].open; 
                entry_notional[c] = current_notional;
            }
        }


        
        if (capital <= 0) { printf("BANKRUPT at day %d\n", d); break; }
    }

    for (int c = 0; c < N_COINS; c++) {
        if (pos[c] == 1 && capital > 0) {
            float pnl = (daily_data[c][total_days-1].close - entry[c]) / entry[c] * entry_notional[c];
            float fee = entry_notional[c] * FEE_PCT * 2.0f;
            capital += (pnl - fee); coin_pnl[c] += (pnl - fee); 
            coin_trades[c]++; if (pnl > fee) coin_wins[c]++;
        }
    }

    printf("\nDetailed Performance per Coin (1H Path):\n");
    printf("-----------------------------------------------------------\n");
    printf("  COIN   TRADES  WINS   WR(%%)    PnL Estim.\n");
    printf("-----------------------------------------------------------\n");
    int total_t = 0, total_w = 0;
    for (int c = 0; c < N_COINS; c++) {
        float wr = coin_trades[c] > 0 ? (float)coin_wins[c]/coin_trades[c]*100 : 0;
        printf("  %-5s  %4d  %4d   %5.1f%%   $%+6.2f\n", coins[c], coin_trades[c], coin_wins[c], wr, coin_pnl[c]);
        total_t += coin_trades[c]; total_w += coin_wins[c];
    }
    printf("-----------------------------------------------------------\n");
    printf("  TOTAL  %4d  %4d   %5.1f%%   Balance: $%.2f\n", total_t, total_w, total_t > 0 ? (float)total_w/total_t*100 : 0, capital);
    printf("-----------------------------------------------------------\n");
    
    return 0;
}
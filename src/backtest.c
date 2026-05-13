#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "include/inference.h"

// Custom struct for backtesting
typedef struct { 
    double open, high, low, close, volume; 
    long long timestamp; 
} OHLC;

#define N_COINS 8
#define START_CAPITAL 20.0f
#define TRADE_MARGIN 2.5f
#define LEVERAGE 10.0f
#define SL_PCT 0.01f

int fetch_ohlc(const char *symbol, OHLC *out, int limit) {
    SSLConnection c = create_ssl_connection(BINANCE_HOST);
    if (!c.ssl) return 0;
    char req[512], buf[8192], *p, *start, *end;
    sprintf(req, "GET /api/v3/klines?symbol=%s&interval=1d&limit=%d HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", symbol, limit, BINANCE_HOST);
    SSL_write(c.ssl, req, strlen(req));
    int n = 0, len, pos = 0, head = 0;
    while (n < limit && (len = SSL_read(c.ssl, buf + pos, sizeof(buf) - pos - 1)) > 0) {
        buf[pos += len] = 0; p = buf;
        if (!head) {
            if (!(p = strstr(buf, "\r\n\r\n"))) continue;
            p += 4; head = 1;
        }
        while (n < limit && (start = strstr(p, "["))) {
            if (start[1] == '[') { p = start + 1; continue; }
            if (!(end = strstr(start, "]"))) break;
            char so[32], sh[32], sl[32], sc[32], sv[32];
            if (sscanf(start + 1, "%lld,\"%[^\"]\",\"%[^\"]\",\"%[^\"]\",\"%[^\"]\",\"%*[^\"]\",%*d,\"%[^\"]\"", &out[n].timestamp, so, sh, sl, sc, sv) >= 6) {
                out[n].open = atof(so); out[n].high = atof(sh); out[n].low = atof(sl); out[n].close = atof(sc); out[n++].volume = atof(sv);
            }
            p = end + 1;
        }
        pos = buf + pos - p; memmove(buf, p, pos);
    }
    cleanup_ssl_connection(c);
    return n;
}

int main() {
    const char *coins[N_COINS] = {"BTC", "ETH", "BNB", "XRP", "SOL", "DOGE", "SHIB", "AVAX"};
    OHLC data[N_COINS][500];
    uint8_t model_bufs[N_COINS][32768];
    Model *models[N_COINS];
    
    printf("--- Shrimp Unified Backtest (8 Coins - Long Only) ---\n");
    printf("Leverage: %.0fx | Shared Capital: $%.2f | Margin: $%.2f\n", LEVERAGE, START_CAPITAL, TRADE_MARGIN);
    
    int coin_days[N_COINS];
    int min_days = 999;
    
    for (int c = 0; c < N_COINS; c++) {
        char symbol[32]; sprintf(symbol, "%sUSDT", coins[c]);
        int n = fetch_ohlc(symbol, data[c], 400);
        if (n < SEQ_LEN + 31) { printf("Error: Not enough data for %s (%d)\n", coins[c], n); return 1; }
        coin_days[c] = n;
        if (n < min_days) min_days = n;
        
        int m_len = download_model_from_github(coins[c], model_bufs[c], 32768);
        model_set_pool(c);
        models[c] = load_model(model_bufs[c], m_len);
        if (!models[c]) { printf("Error: Model load failed for %s\n", coins[c]); return 1; }
    }
    
    // Synchronize: Ensure all coins end at the same relative time
    for (int c = 0; c < N_COINS; c++) {
        if (data[c][coin_days[c]-1].timestamp != data[0][coin_days[0]-1].timestamp) {
            printf("Warning: Timestamp mismatch for %s. Backtest might be unsynced!\n", coins[c]);
        }
    }
    int total_days = min_days;
    
    float capital = START_CAPITAL;
    int pos[N_COINS] = {0}; 
    float entry[N_COINS] = {0}, entry_notional[N_COINS] = {0}, coin_pnl[N_COINS] = {0};
    int coin_trades[N_COINS] = {0}, coin_wins[N_COINS] = {0}, coin_sl[N_COINS] = {0};

    int start_idx = total_days - 31;
    for (int i = start_idx; i < total_days - 1; i++) {
        for (int c = 0; c < N_COINS; c++) {
            float input[SEQ_LEN];
            for (int j = 0; j < SEQ_LEN; j++) {
                int idx = i - SEQ_LEN + 1 + j;
                input[j] = logf((float)data[c][idx].close / (float)data[c][idx-1].close);
            }
            float pred = predict(models[c], input);
            int trend = (pred > 0) ? 1 : 0; 
            
            OHLC today = data[c][i+1];
            if (pos[c] == 1 && trend == 0) {
                float pnl = (today.open - entry[c]) / entry[c] * entry_notional[c];
                capital += pnl; coin_pnl[c] += pnl; coin_trades[c]++; if (pnl > 0) coin_wins[c]++;
                pos[c] = 0;
            }
            if (pos[c] == 0 && trend == 1) {
                float current_margin = capital / N_COINS;
                float current_notional = current_margin * LEVERAGE;
                if (current_notional > today.volume * 0.01f) current_notional = today.volume * 0.01f;
                pos[c] = 1; entry[c] = today.open; entry_notional[c] = current_notional;
            }
            if (pos[c] == 1 && today.low <= entry[c] * (1.0f - SL_PCT)) {
                float loss = entry_notional[c] * SL_PCT;
                capital -= loss; coin_pnl[c] -= loss;
                coin_sl[c]++; coin_trades[c]++; pos[c] = 0;
            }
        }
        if (capital <= 0) break;
    }

    for (int c = 0; c < N_COINS; c++) {
        if (pos[c] == 1 && capital > 0) {
            float pnl = (data[c][total_days-1].close - entry[c]) / entry[c] * entry_notional[c];
            capital += pnl; coin_pnl[c] += pnl; coin_trades[c]++; if (pnl > 0) coin_wins[c]++;
        }
    }

    printf("\nDetailed Performance per Coin:\n");
    printf("-----------------------------------------------------------\n");
    printf("  COIN   TRADES  WINS  SL   WR(%%)    PnL Estim.\n");
    printf("-----------------------------------------------------------\n");
    int total_t = 0, total_w = 0, total_s = 0;
    for (int c = 0; c < N_COINS; c++) {
        float wr = coin_trades[c] > 0 ? (float)coin_wins[c]/coin_trades[c]*100 : 0;
        printf("  %-5s  %4d  %4d  %2d   %5.1f%%   $%+6.2f\n", coins[c], coin_trades[c], coin_wins[c], coin_sl[c], wr, coin_pnl[c]);
        total_t += coin_trades[c]; total_w += coin_wins[c]; total_s += coin_sl[c];
    }
    printf("-----------------------------------------------------------\n");
    printf("  TOTAL  %4d  %4d  %2d   %5.1f%%   Balance: $%.2f\n", total_t, total_w, total_s, (float)total_w/total_t*100, capital);
    printf("-----------------------------------------------------------\n");
    
    return 0;
}

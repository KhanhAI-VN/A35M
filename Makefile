CC = gcc
CFLAGS = -O3 -Wall -I./src/include
LDFLAGS = -lssl -lcrypto -lm -lmicrohttpd

a35m: src/main.c src/include/models.h src/include/openSSL.h src/include/inference.h
	$(CC) $(CFLAGS) src/main.c -o a35m $(LDFLAGS)

backtest: src/backtest.c src/include/models.h src/include/openSSL.h src/include/inference.h
	$(CC) $(CFLAGS) src/backtest.c -o backtest $(LDFLAGS)

clean:
	rm -f a35m backtest btc_predict web_server

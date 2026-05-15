CC = gcc
CFLAGS = -O3 -Wall -I./src/include -I./3libs
LDFLAGS = -lssl -lcrypto -lm -lmicrohttpd

a35m: src/main.c 3libs/sds.c 3libs/cJSON.c src/include/models.h src/include/openSSL.h src/include/inference.h
	$(CC) $(CFLAGS) src/main.c 3libs/sds.c 3libs/cJSON.c -o a35m $(LDFLAGS)

.PHONY: backtest

backtest: src/backtest.c 3libs/sds.c 3libs/cJSON.c src/include/models.h src/include/openSSL.h src/include/inference.h
	$(CC) $(CFLAGS) src/backtest.c 3libs/sds.c 3libs/cJSON.c -o backtest $(LDFLAGS)
	./backtest

clean:
	rm -f a35m backtest

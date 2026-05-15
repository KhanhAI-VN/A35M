CC = gcc
CFLAGS = -O3 -Wall -I./src/include -I./3libs
LDFLAGS = -lcurl -lm -lmicrohttpd

a35m: src/main.c 3libs/sds.c 3libs/cJSON.c 3libs/log.c 3libs/hashmap.c src/include/models.h src/include/inference.h
	$(CC) $(CFLAGS) src/main.c 3libs/sds.c 3libs/cJSON.c 3libs/log.c 3libs/hashmap.c -o a35m $(LDFLAGS)

.PHONY: backtest

backtest: src/backtest.c 3libs/sds.c 3libs/cJSON.c 3libs/log.c 3libs/hashmap.c src/include/models.h src/include/inference.h
	$(CC) $(CFLAGS) src/backtest.c 3libs/sds.c 3libs/cJSON.c 3libs/log.c 3libs/hashmap.c -o backtest $(LDFLAGS)
	./backtest

clean:
	rm -f a35m backtest

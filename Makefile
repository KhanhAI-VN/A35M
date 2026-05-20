CC = gcc
CFLAGS = -O3 -Wall -I./src/include -I./3libs
LDFLAGS = -lcurl -lm -lmicrohttpd -luv

a35m: tools/patch.py
	python3 tools/patch.py
	$(CC) $(CFLAGS) core.c -o a35m $(LDFLAGS)

.PHONY: backtest

backtest: src/backtest.c 3libs/sds.c 3libs/yyjson.c 3libs/log.c 3libs/hashmap.c 3libs/safe_cache.c src/include/models.h src/include/inference.h
	$(CC) $(CFLAGS) src/backtest.c 3libs/sds.c 3libs/yyjson.c 3libs/log.c 3libs/hashmap.c 3libs/safe_cache.c -o backtest $(LDFLAGS)
	./backtest

test_conf: src/test_conf.c 3libs/sds.c 3libs/yyjson.c 3libs/log.c 3libs/hashmap.c 3libs/safe_cache.c src/include/models.h src/include/inference.h
	$(CC) $(CFLAGS) src/test_conf.c 3libs/sds.c 3libs/yyjson.c 3libs/log.c 3libs/hashmap.c 3libs/safe_cache.c -o test_conf $(LDFLAGS)
	./test_conf BTC

clean:
	rm -f a35m backtest test_conf core.c

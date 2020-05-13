CFLAGS = -g
LIB = lib/*.c

linear_probe_test: main.c linear_probe.c $(LIB)
	gcc $(CFLAGS) -o $@ $^

bitset_test: main.c bitset.c $(LIB)
	gcc $(CFLAGS) -o $@ $^

robinhood_test: main.c robinhood_hashing.c $(LIB)
	gcc $(CFLAGS) -o $@ $^

EBVBL_test: main.c EBVBL.c $(LIB)
	gcc $(CFLAGS) -o $@ $^

clean:
	-rm *_test
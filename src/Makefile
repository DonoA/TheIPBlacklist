CFLAGS = -g
LIB = lib/*.c

linear_probe_test: main.c linear_probe.c $(LIB)
	gcc $(CFLAGS) -o $@ $^

bitset_test: main.c bitset.c $(LIB)
	gcc $(CFLAGS) -o $@ $^

clean:
	-rm *_test
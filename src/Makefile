CFLAGS = -g
LIB = allocator.c common.c fast_rand.c vector.c subnet_loader.c

linear_probe_test: main.c linear_probe.c $(LIB)
	gcc $(CFLAGS) -o $@ $^

clean:
	-rm *_test
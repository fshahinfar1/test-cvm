build_dir = ./build
binary = $(build_dir)/test

CFLAGS = -O3 -g

.PHONY: default

default: $(binary)
	./build/test

$(binary): test.c treap_test.h ../treap.h
	if [ ! -d $(build_dir) ]; then mkdir -p $(build_dir); fi
	$(CC) $(CFLAGS) -o $@ test.c $(LDFLAGS)


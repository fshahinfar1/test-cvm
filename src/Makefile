build_dir = ./build
binary = $(build_dir)/main

CFLAGS = -O3 -g

.PHONY: default
default: $(binary)
$(binary): cvm.c treap/treap.h
	if [ ! -d $(build_dir) ]; then mkdir -p $(build_dir); fi
	$(CC) $(CFLAGS) -o $@ cvm.c $(LDFLAGS)

build_dir = ./build
binary = $(build_dir)/main

CFLAGS = -O3 -g -Wall

.PHONY: default
default: $(binary)

clean:
	rm -r $(build_dir)

$(binary): cvm.c treap/treap.h fixed_point/fp.h
	if [ ! -d $(build_dir) ]; then mkdir -p $(build_dir); fi
	$(CC) $(CFLAGS) -o $@ cvm.c $(LDFLAGS)

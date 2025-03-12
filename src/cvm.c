/* An implementation fo CVM algorithm for estimating the number of distinct
 * elements in a stream.
 *
 * @author: Farbod Shahinfar
 * @date: March, 2025
 * */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

// The buffer size we want to use for CVM (|B| in the Knuth's CVM paper)
#define TREAP_MAX_SIZE 128
#define TREAP_MAX_HEIGHT 16

#include "treap/treap.h"
#include "fixed_point/fp.h"

struct cvm_impl {
	struct treap *t;
	fp_t p;
};

struct cvm_impl *cvm_new()
{
	// allocating the CVM struct and treap struct
	size_t sz = sizeof(struct cvm_impl) + sizeof(struct treap);
	struct cvm_impl *c = calloc(1, sz);
	if (c == NULL)
		return NULL;
	c->t = (struct treap *)(c + 1);
	c->p = FP_ONE;
	return c;
}

uint64_t cvm_estimate(struct cvm_impl *cvm)
{
	return cvm->t->used * fp_recerpical_to_uint(cvm->p);
}

int main(int argc, char *argv[])
{
	struct cmv_impl *cvm = cvm_new();

	// read the stream of keys from a file
	FILE *f = fopen("./test/data.txt", "r");
	assert(f != NULL);
	struct treap_key key;
	int line_counter = 0;
	while (fscanf(f, "%d", (int *)&key.data) > 0) {
		printf("%d: %d\n", line_counter++, *(int *)key.data);
	}
	return 0;
}

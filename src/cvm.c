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
#include <time.h>

// The buffer size we want to use for CVM (|B| in the Knuth's CVM paper)
#define TREAP_MAX_SIZE (1 << 10)
#define TREAP_MAX_HEIGHT 32

#include "treap/treap.h"
#include "fixed_point/fp.h"

struct cvm_impl {
	struct treap *t;
	fp_t p;
};

struct cvm_impl *cvm_new()
{
	// allocating the CVM struct and treap struct
	struct cvm_impl *c = calloc(1, sizeof(struct cvm_impl));
	if (c == NULL) return NULL;
	c->t = treap_new();
	if (c->t == NULL) {
		free(c);
		return NULL;
	}
	c->p = FP_ONE;
	return c;
}

uint64_t cvm_estimate(struct cvm_impl *cvm)
{
	return (uint64_t)((double)cvm->t->used / fp_to_float(cvm->p));
}

int main(int argc, char *argv[])
{
	srand(time(0));
	struct cvm_impl *cvm = cvm_new();
	int ret;

	// read the stream of keys from a file
	FILE *f = fopen("./test/data.txt", "r");
	assert(f != NULL);
	struct treap_key key;
	/* int line_counter = 0; */
	while (fscanf(f, "%d", (int *)&key.data) > 0) {
		/* printf("%d: %d\n", line_counter++, *(int *)key.data); */

		// if we have the element in the buffer, remove it
		ret = treap_delete(cvm->t, &key);
		/* if (ret == 0) { */
		/* 	printf("delete key: %d\n", *(uint32_t *)key.data); */
		/* } */
		fp_t u = fp_random();
		/* printf("rand: %f | %f\n", fp_to_float(u), 1/fp_to_float(u)); */
		if (u >= cvm->p)
			continue;
		if (treap_has_space(cvm->t)) {
			ret = treap_insert(cvm->t, &key, u);
			assert(ret == 0);
			continue;
		}
		// u < p and |B| = s
		assert(u < cvm->p);
		assert(cvm->t->used == TREAP_MAX_SIZE);
		struct treap_node *top = treap_top(cvm->t);
		if (u > top->priority) {
			/* printf("(1) p: %f --> %f\n", fp_to_float(cvm->p), fp_to_float(u)); */
			cvm->p = u;
		} else {
			/* printf("(2) p: %f --> %f\n", fp_to_float(cvm->p), fp_to_float(top->priority)); */
			cvm->p = top->priority;
			// TODO: I may be able to implement a better node replace code than remove and add
			/* printf("old top key: %d (%f)\n", *(uint32_t *)&top->key, fp_to_float(top->priority)); */
			/* top->priority = u; */
			/* memcpy(top->key.data, key.data, TREAP_KEY_SIZE); */
			/* printf("new top key: %d (%f)\n", *(uint32_t *)&top->key, fp_to_float(top->priority)); */

			ret = treap_delete(cvm->t, &top->key);
			assert(ret == 0);
			ret = treap_insert(cvm->t, &key, u);
			if (ret == -2)
				printf("height issue: %d\n", ret);
			assert(ret == 0);
		}
	}
	printf("|B|: %d   p: %f\n", cvm->t->used, fp_to_float(cvm->p));
	printf("Estimate: %ld\n", cvm_estimate(cvm));
	return 0;
}

#pragma once
#include <assert.h>
#include "../treap.h"

#ifndef bool
typedef uint8_t bool;
enum bool_values {
	false = 0,
	true = 1,
};
#endif

static bool treap_valid(struct treap_node *ptr)
{
	if (ptr == NULL)
		return true;

	// check the left and right subtree to be valid
	if (!treap_valid(ptr->left) || !treap_valid(ptr->right)) {
		return false; // there was an issue
	}

	if (ptr->left != NULL) {
		// every left child should be strictly smaller than its parent
		if (!treap_key_less_than(&ptr->left->key, &ptr->key)) {
			fprintf(stderr, "%d < %d\n", *(uint32_t *)ptr->left->key.data,
					*(uint32_t *)ptr->key.data);
			return false;
		}

		// parent should have a higher or equal priority
		if (ptr->priority < ptr->left->priority) {
			fprintf(stderr, "priority violation (1)\n");
			return false;
		}
	}

	if (ptr->right != NULL) {
		// every right parent should be less or equal to the right child
		if (!treap_key_less_than(&ptr->key, &ptr->right->key))
			if (!treap_key_eq(&ptr->key, &ptr->right->key)) {
				fprintf(stderr, "%d < %d",
					*(uint32_t *)ptr->key.data,
					*(uint32_t *)ptr->right->key.data);
				return false;
			}

		// parent should have a higher or equal priority
		if (ptr->priority < ptr->right->priority) {
			fprintf(stderr, "priority violation (2)\n");
			return false;
		}
	}

	// found no issue (note: an empty treap is also valid)
	return true;
}

static int treap_report_priority_in_order(struct treap *t, int **out, uint32_t *out_sz)
{
	// It is a DFS reporting left, parent, right nodes :)

	int count_nodes = t->used;
	int *arr = calloc(count_nodes, sizeof(int));
	if (arr == NULL)
		return -ENOMEM;
	*out = arr;
	*out_sz = count_nodes;
	int arr_index = 0;

	if (count_nodes == 0)
		return 0;

	struct treap_node *stack[TREAP_MAX_HEIGHT] = {};
	size_t stack_sz = 0;

	struct treap_node *cur = t->root;
	while (stack_sz > 0 || cur != NULL) {
		if (cur != NULL) {
			// stack:push
			stack[stack_sz++] = cur;
			cur = cur->left;
		} else {
			// stack:pop
			stack_sz--;
			cur = stack[stack_sz];
			// visit
			arr[arr_index++] = cur->priority;
			cur = cur->right;
		}
	}

	return 0;
}

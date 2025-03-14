#pragma once
/* *
 * a Treap data structure implementation
 * author: Farbod Shahinfar
 * LICENSE: MIT
 * */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define __packed __attribute__((packed))
#ifndef __always_inline
#define __always_inline __attribute__((always_inline))
#endif


#ifndef TREAP_MAX_SIZE
#define TREAP_MAX_SIZE 128
#endif

#ifndef TREAP_MAX_HEIGHT
// the max height is used to make while loops, ... bounded
#define TREAP_MAX_HEIGHT 16
#endif

// user can define its own key by defining the key size and following functions
// and struct (NOTE: it looks ugly do something better later)
#ifndef TREAP_KEY_SIZE
#define TREAP_KEY_SIZE 4
struct treap_key {
	uint8_t data[TREAP_KEY_SIZE];
} __packed;

static __always_inline
int treap_key_less_than(struct treap_key *a, struct treap_key *b)
{
	return *((uint32_t *)&a->data) < *((uint32_t *)&b->data);
}

static __always_inline
int treap_key_eq(struct treap_key *a, struct treap_key *b)
{
	return *((uint32_t *)&a->data) == *((uint32_t *)&b->data);
}
#endif

struct treap_node {
	struct treap_key key;
	uint32_t priority;
	struct treap_node *left;
	struct treap_node *right;
};

struct treap {
	struct treap_node *root;
	struct treap_node nodes[TREAP_MAX_SIZE];
	uint32_t used; // number of nodes in the treap --> Top of the stack (TREAP_MAX_SIZE - used - 1)
	struct treap_node *stack[TREAP_MAX_SIZE]; // stack of free nodes
};

struct treap *treap_new(void)
{
	// calloc will initialize the left and rgith pointers to be NULL, which is
	// good
	struct treap *t = calloc(1, sizeof(struct treap));
	if (t == NULL)
		return NULL;
	for (uint32_t k = 0; k < TREAP_MAX_SIZE; k++)
		t->stack[TREAP_MAX_SIZE - k - 1] = &t->nodes[k];
	return t;
}

void treap_destroy(struct treap *t)
{
	free(t);
}

/* get the highest priority node (a.k.a. root)
 * */
struct treap_node *treap_top(struct treap *t) {
	return t->root;
}

static void __treap_find(struct treap *t, struct treap_key *key,
		struct treap_node **node_out,
		struct treap_node ***node_parent_link)
{
	*node_out = NULL;
	*node_parent_link = NULL;

	struct treap_node *ptr = t->root;
	struct treap_node **link = &t->root;
	uint32_t k;
	for (k = 0; k < TREAP_MAX_HEIGHT; k++) {
		if (ptr == NULL) {
			// key does not exists in the treap (or there is a bug in
			// implementation of the treap)
			return;
		}
		// TODO: maybe I could optimize it with having only one comparison
		if (treap_key_less_than(key, &ptr->key)) {
			// less
			link = &ptr->left;
			ptr = ptr->left;
		} else {
			// greater or equal
			if (treap_key_eq(key, &ptr->key)) {
				// found it
				*node_parent_link = link;
				*node_out = ptr;
				return;
			}
			link = &ptr->right;
			ptr = ptr->right;
		}
	}
	// did not found the result in the bounded height
	return;
}

/* fint the node with the given key
 * */
struct treap_node *treap_find(struct treap *t, struct treap_key *key)
{
	struct treap_node *n;
	struct treap_node **link;
	__treap_find(t, key, &n, &link);
	return n;
}

enum ROTATE_DIR {
	LEFT,
	RIGHT,
};

static __always_inline
struct treap_node ** __get_parent_link(struct treap_node *p, struct treap_node *n)
{
	if (p->left == n) {
		return &p->left;
	} else if (p->right == n) {
		return &p->right;
	}
	return NULL;
}

static __always_inline
void __rotate(struct treap_node **link, enum ROTATE_DIR dir)
{
	// these names represents the initial state, after rotation the parent will
	// become the child and child will be the parent
	struct treap_node *parent, *child;

	parent = *link;
	/* printf("rotate %s on node with prio=%d\n", */
	/* 		dir == LEFT ? "left": "right", */
	/* 		parent == NULL ? -1 : parent->priority); */

	if (parent == NULL) {
		// nothing to do
		return;
	}

	if (dir == RIGHT) {
		child = parent->left;
		if (child == NULL) {
			// nothing to do
			return;
		}

		*link = child;
		struct treap_node *left_right = child->right;
		child->right = parent;
		parent->left = left_right;
	} else if (dir == LEFT) {
		child = parent->right;
		if (child == NULL) {
			// nothing to do
			return;
		}

		*link = child;
		struct treap_node *right_left = child->left;
		child->left = parent;
		parent->right = right_left;
	}
}

static __always_inline
struct treap_node * __treap_alloc_node(struct treap *t)
{
	if (t->used >= TREAP_MAX_SIZE) {
		// pool of nodes has been exausted
		return NULL;
	}
	uint32_t top_stack = TREAP_MAX_SIZE - t->used -1;
	t->used++;
	struct treap_node *new = t->stack[top_stack];
	// initialize
	new->left = new->right = NULL;
	return new;
}

static __always_inline
void __treap_free_node(struct treap *t, struct treap_node *n)
{
	// NOTE: we are not checking if the give node is actually valid pointer.
	// Becareful!
	if (t->used <= 0) {
		// Something is very wrong
		return;
	}

	t->used--;
	uint32_t top_stack = TREAP_MAX_SIZE - t->used - 1;
	t->stack[top_stack] = n;
}

int treap_insert(struct treap *t, struct treap_key *k, uint32_t priority)
{
	// get a node
	struct treap_node *n = __treap_alloc_node(t);
	if (n == NULL) {
		return -ENOSPC;
	}
	memcpy(&n->key, k, sizeof(struct treap_key));
	n->priority = priority;

	// find the right place on the binary tree
	struct treap_node *ptr = t->root;
	struct treap_node **link = &t->root;

	uint32_t i; // path len
	struct treap_node *path[TREAP_MAX_HEIGHT] = {};

	// I want a bounded loop since I am planing to use it in eBPF
	for (i = 0; i < TREAP_MAX_HEIGHT; i++) {
		if (ptr == NULL)
			break;
		path[i] = ptr;
		if (treap_key_less_than(k, &ptr->key)) {
			link = &ptr->left;
			ptr = ptr->left;
		} else {
			link = &ptr->right;
			ptr = ptr->right;
		}
	}
	// did not found the empty space in the bounded height
	if (i >= TREAP_MAX_HEIGHT) {
		printf("%d > %d\n", i, TREAP_MAX_HEIGHT);
		t->used--; // free the node we reserved
		return -2;
	}

	// assign the node to the empty place we found
	*link = n;

	/* if (i == 0) { */
	/* 	// The treap was empty and we just set the root */
	/* 	// The heap property is valid */
	/* 	return 0; */
	/* } */

	int parent_index = i;
	for (uint32_t k = 0; k < TREAP_MAX_HEIGHT; k++) {
		if (parent_index <= 0) {
			// we reached the root (rotate the node to the root)
			break;
		}
		parent_index--;
		struct treap_node *p = path[parent_index];
		struct treap_node **grand_p_link = NULL;
		if (n->priority > p->priority) {
			// do rotation
			if (parent_index > 0)
				grand_p_link = __get_parent_link(path[parent_index - 1], p);
			else
				grand_p_link = &t->root;
			if (p->left == n) {
				// do a right rotation rooted at parent
				__rotate(grand_p_link, RIGHT);
			} else {
				// do a left rotation rooted at parent
				__rotate(grand_p_link, LEFT);
			}
		}
	}
	return 0;
}

struct treap_node *__get_imidiate_succesor(struct treap_node *n,
		struct treap_node ***out_link)
{
	struct treap_node *leaf = n->right;
	struct treap_node **link = &n->right;
	uint32_t k;
	if (n->right == NULL)
		return NULL;
	for (k = 0; k < TREAP_MAX_HEIGHT; k++) {
		if (leaf->left == NULL)
			break;
		link = &leaf->left;
		leaf = leaf->left;
	}
	if (k >= TREAP_MAX_HEIGHT) {
		// failed to do it in a bounded size
		return NULL;
	}
	*out_link = link;
	return leaf;
}

// Bubble down the node fixing the heap property
int __fix_sub_tree_heap_property_down(struct treap_node *ptr, struct treap_node **ptr_link)
{
	// The state of the treap is as follows:
	//   * the binary search property is valid
	//   * the heap property is possiblly not, because we moved replace a node
	//     with its next successor

	// How to fix the heap property ?
	// Each left and right sub-tree of replaced node are valid treaps.
	// 1. If the node priority is more than both, we are good
	// 2. Otherwise, rotate the node toward the sub-tree with lower priority
	//    e.g., if the priority for the top of sub-tree is 5 and for the right is 8, we rotate left.
	// 3. Goto 1! [Repeat until we exit at step 1].

	// Why it works? at each rotation, the node with higher priority will
	// bubble up ... (not 100% sure actually)


	uint32_t p = ptr->priority;
	uint32_t k;
	// we do not need to update the ptr, the ptr is the node we want to
	// buble down.
	for (k = 0; k < TREAP_MAX_HEIGHT; k++) {
		uint32_t left_p, right_p;
		if (ptr->left == NULL) {
			if (ptr->right == NULL) {
				// we are good
				break;
			} else if (p >= ptr->right->priority) {
				// we are good
				break;
			} else {
					__rotate(ptr_link, LEFT);
					ptr_link = &((*ptr_link)->left);
			}
		} else if (ptr->right == NULL) {
			// NOTE: we know the left is not null
			left_p = ptr->left->priority;
			if (p >= ptr->left->priority) {
				// we are good
				break;
			} else {
					__rotate(ptr_link, RIGHT);
					ptr_link = &((*ptr_link)->right);
			}
		} else {
			left_p = ptr->left->priority;
			right_p = ptr->right->priority;
			if (p >= left_p) {
				if (p >= right_p) {
					// we are good
					break;
				} else {
					// right_p > left_p --> rotate to the side with lower priority (LEFT)
					__rotate(ptr_link, LEFT);
					ptr_link = &((*ptr_link)->left);
				}
			} else {
				// left_p > p
				if (left_p <= right_p) {
					__rotate(ptr_link, LEFT);
					ptr_link = &((*ptr_link)->left);
				} else {
					__rotate(ptr_link, RIGHT);
					ptr_link = &((*ptr_link)->right);
				}
			}
		}

		/* if ((ptr->right != NULL) && */
		/* 		(ptr->priority < ptr->right->priority)) { */
		/* 	__rotate(ptr_link, LEFT); */
		/* 	ptr_link = &((*ptr_link)->left); */
		/* } else if ((ptr->left != NULL) && */
		/* 		(ptr->priority < ptr->left->priority)) { */
		/* 	__rotate(ptr_link, RIGHT); */
		/* 	ptr_link = &((*ptr_link)->right); */
		/* } else { */
		/* 	// everything is good */
		/* 	break; */
		/* } */
	}
	if (k >= TREAP_MAX_HEIGHT) {
		fprintf(stderr, "error: could not fix tree in the bounded number of iterations\n");
		return -1;
	}
	return 0;
}

int treap_delete(struct treap *t, struct treap_key *key)
{
	int ret;
	struct treap_node *n;
	struct treap_node **link;
	__treap_find(t, key, &n, &link);
	if (n == NULL) {
		// key does not exist
		return -1;
	}
	if (n->left == NULL) {
		if (n->right == NULL) {
			// it is a leaf, just remove the node
			*link = NULL;
		} else {
			// has one child (the right child)
			*link = n->right;
		}
	} else {
		// has left child
		if (n->right == NULL) {
			// has one child (the left child)
			*link = n->left;
		} else {
			// We need to move the node down, find the imidiate
			// successor (the left most left node of the right
			// sub-tree)
			struct treap_node **leaf_link = NULL;
			struct treap_node *leaf = __get_imidiate_succesor(n, &leaf_link);
			if (leaf == NULL) {
				// failed to do it in a bounded size
				return -2;
			}

			// swap the node with the leaf and then remove the node
			// .. we know leaf does not have a left, if it does not
			// have a right then just remove the node. if it has a
			// right child then put the right child in place of
			// leaf.
			*leaf_link = leaf->right;
			leaf->right = n->right;
			leaf->left = n->left;
			*link = leaf;
			// node has been removed and everything is almost okay
			// except that moving leaf to the nodes position may
			// have disturbed the heap property
			ret = __fix_sub_tree_heap_property_down(leaf, link);
			assert(ret == 0);
		}
	}

	// return the node to the stack of free nodes :)
	__treap_free_node(t, n);
	return 0;
}

static __always_inline
uint8_t treap_has_space(struct treap *t)
{
	return t->used < TREAP_MAX_SIZE;
}

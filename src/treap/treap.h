#pragma once
/* *
 * a Treap data structure implementation
 * author: Farbod Shahinfar
 * LICENSE: MIT
 * */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define __packed __attribute__((packed))
#ifndef __always_inline
#define __always_inline __attribute__((always_inline))
#endif

#define KEY_SIZE 4
#define TREAP_MAX_SIZE 128
// the max height is used to make while loops, ... bounded
#define TREAP_MAX_HEIGHT 32

struct treap_key {
	uint8_t data[KEY_SIZE];
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

struct treap_node {
	struct treap_key key;
	uint32_t priority;
	struct treap_node *left;
	struct treap_node *right;
};

struct treap {
	struct treap_node *root;
	struct treap_node nodes[TREAP_MAX_SIZE];
	uint32_t used;
};

struct treap *treap_new(void)
{
	// calloc will initialize the left and rgith pointers to be NULL, which is
	// good
	struct treap *t = calloc(1, sizeof(struct treap));
	if (t == NULL)
		return NULL;
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

/* fint the node with the given key
 * */
struct treap_node *treap_find(struct treap *t, struct treap_key *key)
{
	struct treap_node *ptr = t->root;
	uint32_t k;
	for (k = 0; k < TREAP_MAX_HEIGHT; k++) {
		if (ptr == NULL) {
			// key does not exists in the treap (or there is a bug in
			// implementation of the treap)
			return NULL;
		}
		// TODO: maybe I could optimize it with having only one comparison
		if (treap_key_less_than(key, &ptr->key)) {
			// less
			ptr = ptr->left;
		} else {
			// greater or equal
			if (treap_key_eq(key, &ptr->key)) {
				// found it
				return ptr;
			}
			ptr = ptr->right;
		}
	}
	// did not found the result in the bounded height
	return NULL;
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

int treap_insert(struct treap *t, struct treap_key *k, uint32_t priority)
{
	if (t->used >= TREAP_MAX_SIZE)
		return -1;

	// get a node
	struct treap_node *n = &t->nodes[t->used++];
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
	if (i >= TREAP_MAX_SIZE) {
		t->used--; // free the node we reserved
		return -ENOSPC;
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
		if (parent_index < 0) {
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

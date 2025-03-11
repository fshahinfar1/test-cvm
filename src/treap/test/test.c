#include <stdio.h>
#include "treap_test.h"

#define ASSERT(cond, ...) \
	if (!(cond)) { \
		printf("@line %d: ", __LINE__); \
		printf(__VA_ARGS__); \
		printf("\n"); \
		return false; \
	}

bool test_basic(void)
{
	struct treap_key K = {};
	struct treap *t = treap_new();
	ASSERT(t != NULL, "failed to allocate a treap");
	ASSERT(t->root == NULL, "newly allocated treap has non-NULL value as root pointer");

	*(uint32_t *)(&K.data) = 123;
	treap_insert(t, &K, 100);
	struct treap_node *root = t->root;
	ASSERT(root != NULL, "root pointer is null after insrting a value");
	ASSERT(treap_key_eq(&t->root->key, &K), "key inside the root does not match the key we inserted");
	ASSERT(root->priority == 100, "priority does not match what we inserted (%d != %d)", root->priority, 100);
	ASSERT(root->left == NULL, "the left child is not null, we have inserted only one node");
	ASSERT(root->right == NULL, "the right child is not null, we have inserted only one node");

	// This node should become the right child of the current
	*(uint32_t *)(&K.data) = 321;
	treap_insert(t, &K, 50);
	ASSERT(t->root == root, "the root pointer changed althoug it should have not");
	ASSERT(root->left == NULL, "the left child should be null");
	struct treap_node *right = root->right;
	ASSERT(right != NULL, "the right child should not be null");
	ASSERT(right->priority == 50, "wrong priority");
	ASSERT(treap_key_eq(&right->key, &K), "key did not matched");

	// This should become the left child of the right child
	*(uint32_t *)(&K.data) = 200;
	treap_insert(t, &K, 38);
	ASSERT(t->root == root, "the root pointer changed althoug it should have not");
	ASSERT(root->left == NULL, "the left child should be null");
	ASSERT(root->right == right, "right node should have not changed");
	ASSERT(right->right == NULL, "the node should be null");
	struct treap_node *left = right->left;
	ASSERT(left != NULL, "the child is not set!");
	ASSERT(left->priority == 38, "the priority value is wrong");
	ASSERT(treap_key_eq(&left->key, &K), "the key did not match");

	// This should first become the right child of the right child :)
	// but then it should cause two left rotations and become the root
	*(uint32_t *)(&K.data) = 512;
	treap_insert(t, &K, 120);
	struct treap_node *rr = t->root;
	ASSERT(t->root != root, "root should have changed");
	ASSERT(rr->left != NULL, "the left child should not be null" );
	ASSERT(rr->left == root, "the left child should be the old root");
	ASSERT(rr->right == NULL, "the right child should be null");
	ASSERT(root->right == right, "The right child should correctly be moved with root after rotations");

	// check the inorder walk
	int expected_arr[] = {100,38,50,120};
	int *arr; uint32_t arr_sz;
	treap_report_priority_in_order(t, &arr, &arr_sz);
	ASSERT(arr_sz == 4, "there should be 4 nodes");
	for (int k = 0; k < arr_sz; k++) {
		ASSERT(expected_arr[k] == arr[k], "issue in order of nodes");
	}

	treap_destroy(t);
	return true; // pass
}

int main(int argc, char *argv[])
{
	printf("\n\n"
		"===========================================\n"
		"\t\tTESTING\n"
		"...........................................\n");
	bool res;
	res = test_basic();
	if (!res) {
		printf("Test failed\n");
		return -1;
	}
	printf("Test passed\n");
	return 0;
}

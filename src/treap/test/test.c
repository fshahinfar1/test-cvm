#include <stdio.h>
#include "treap_test.h"

#define ASSERT(cond, ...) \
	if (!(cond)) { \
		printf("@line %d: ", __LINE__); \
		printf(__VA_ARGS__); \
		printf("\n"); \
		return false; \
	}

typedef bool(*test_fn)(void);

bool test_basic(void)
{
	int ret;
	struct treap_key K = {};
	struct treap *t = treap_new();
	ASSERT(t != NULL, "failed to allocate a treap");
	ASSERT(t->root == NULL, "newly allocated treap has non-NULL value as root pointer");
	ASSERT(treap_valid(t->root) == 1, "check validity of treap");

	*(uint32_t *)(&K.data) = 123;
	treap_insert(t, &K, 100);
	struct treap_node *root = t->root;
	ASSERT(root != NULL, "root pointer is null after insrting a value");
	ASSERT(treap_key_eq(&t->root->key, &K), "key inside the root does not match the key we inserted");
	ASSERT(root->priority == 100, "priority does not match what we inserted (%d != %d)", root->priority, 100);
	ASSERT(root->left == NULL, "the left child is not null, we have inserted only one node");
	ASSERT(root->right == NULL, "the right child is not null, we have inserted only one node");
	ASSERT(treap_valid(t->root) == 1, "check validity of treap");

	// This node should become the right child of the current
	*(uint32_t *)(&K.data) = 321;
	treap_insert(t, &K, 50);
	ASSERT(t->root == root, "the root pointer changed althoug it should have not");
	ASSERT(root->left == NULL, "the left child should be null");
	struct treap_node *right = root->right;
	ASSERT(right != NULL, "the right child should not be null");
	ASSERT(right->priority == 50, "wrong priority");
	ASSERT(treap_key_eq(&right->key, &K), "key did not matched");
	ASSERT(treap_valid(t->root) == 1, "check validity of treap");

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
	ASSERT(treap_valid(t->root) == 1, "check validity of treap");

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
	ASSERT(treap_valid(t->root) == 1, "check validity of treap");

	*(uint32_t *)(&K.data) = 322;
	treap_insert(t, &K, 37);
	ASSERT(treap_valid(t->root) == 1, "check validity of treap");

	// check the inorder walk
	int expected_arr[8];
	memcpy(expected_arr , ((int[]){100,38,50,37,120}), 5*sizeof(int));
	int *arr; uint32_t arr_sz;
	treap_report_priority_in_order(t, &arr, &arr_sz);
	ASSERT(arr_sz == 5, "number of nodes are wrong");
	for (int k = 0; k < arr_sz; k++) {
		ASSERT(expected_arr[k] == arr[k], "issue in order of nodes");
	}
	free(arr);


	// test deleting a node that does not exists
	*(uint32_t*)(&K.data) = 111;
	ret = treap_delete(t, &K);
	ASSERT(ret == -1, "treap_delete should have failed");
	ASSERT(treap_valid(t->root) == 1, "check validity of treap");

	// test deleting a node (the current root is being removed)
	*(uint32_t*)(&K.data) = 321;
	ret = treap_delete(t, &K);
	ASSERT(ret == 0, "treap_delete failed");
	ASSERT(treap_valid(t->root) == 1, "check validity of treap");

	treap_report_priority_in_order(t, &arr, &arr_sz);
	ASSERT(arr_sz == 4, "number of nodes are wrong");
	memcpy(expected_arr , ((int[]){100,38,37,120}), 4*sizeof(int));
	for (int k = 0; k < arr_sz; k++) {
		ASSERT(expected_arr[k] == arr[k], "issue in order of nodes");
	}
	free(arr);

	ASSERT(t->root->right == NULL, "[missing description]");
	ASSERT(*(uint32_t *)t->root->key.data == 512, "[missing description]");
	ASSERT(t->root->left != NULL,"[missing description]");
	ASSERT(*(uint32_t *)t->root->left->key.data == 123, "[missing description]");
	ASSERT(t->root->left->left == NULL,"[missing description]");
	ASSERT(t->root->left->right != NULL,"[missing description]");
	ASSERT(*(uint32_t *)t->root->left->right->key.data == 200, "[missing description]");
	ASSERT(t->root->left->right->left == NULL,"[missing description]");
	ASSERT(t->root->left->right->right != NULL,"[missing description]");
	ASSERT(*(uint32_t *)t->root->left->right->right->key.data == 322, "[missing description]");

	//
	*(uint32_t*)(&K.data) = 512;
	ret = treap_delete(t, &K);
	ASSERT(ret == 0, "treap_delete failed");
	ASSERT(treap_valid(t->root) == 1, "check validity of treap");
	*(uint32_t*)(&K.data) = 200;
	ret = treap_delete(t, &K);
	ASSERT(ret == 0, "treap_delete failed");
	ASSERT(treap_valid(t->root) == 1, "check validity of treap");
	*(uint32_t*)(&K.data) = 123;
	ret = treap_delete(t, &K);
	ASSERT(ret == 0, "treap_delete failed");
	ASSERT(treap_valid(t->root) == 1, "check validity of treap");
	*(uint32_t*)(&K.data) = 322;
	ret = treap_delete(t, &K);
	ASSERT(ret == 0, "treap_delete failed");
	ASSERT(treap_valid(t->root) == 1, "check validity of treap");
	ASSERT(t->used == 0, "treap should be empty now");

	treap_destroy(t);
	return true; // pass
}

bool test_delete(void)
{
	int ret;
	struct treap_key K = {};
	struct treap *t = treap_new();

	uint32_t test_keys[] = { 3, 4, 1, 2, };
	size_t count_test_keys = sizeof(test_keys) / sizeof(test_keys[0]);

	for (int i = 0; i < count_test_keys; i++) {
		*(uint32_t *)K.data = test_keys[i];
		treap_insert(t, &K, i+1);
	}
	ASSERT(treap_valid(t->root) == 1, "check validity of treap");

	for (int i = 0; i < count_test_keys; i++) {
		*(uint32_t *)K.data = test_keys[count_test_keys - i - 1];
		treap_delete(t, &K);
		ASSERT(treap_valid(t->root) == 1, "check validity of treap (delete: %d)", i);
	}

	return true;
}

int main(int argc, char *argv[])
{
	printf("\n\n"
		"===========================================\n"
		"\t\tTESTING\n"
		"...........................................\n");
	bool res;
	test_fn suite[] = {test_basic, test_delete,};
	const size_t count_tests = sizeof(suite)/sizeof(suite[0]);
	for (int i = 0; i < count_tests; i++) {
		res = suite[i]();
		if (!res) {
			printf("Test %d failed\n", i+1);
			return -1;
		}
	}
	printf("Test passed\n");
	return 0;
}

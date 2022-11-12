#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree-private.h"
#include "tree.h"

#define NUM_OPERATIONS 100

int main();
char* randomString(int length);

int main() {
	struct tree_t* tree = tree_create();

	char** keys = (char**)malloc(NUM_OPERATIONS * sizeof(char*));
	for (int i = 0; i < NUM_OPERATIONS; i++) {
		keys[i] = randomString(10);
		struct data_t* value = data_create2(strlen(keys[i]), strdup(keys[i]));
		tree_put(tree, keys[i], value);
		data_destroy(value);
	}

	//print_tree_id(-1, tree);

	for (int i = 0; i < NUM_OPERATIONS; i++) {
		tree_del(tree, keys[i]);
	}

	printf("AFTER DELETE\n");

	print_tree_id(-1, tree);
}

char* randomString(int length) {
	if (!length)
		return NULL;

	static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
	char* string = (char*)malloc(sizeof(char) * (length + 1));
	if (!string)
		return NULL;

	for (int n = 0; n < length; n++) {
		int key = rand() % (int)(sizeof(charset) - 1);
		string[n] = charset[key];
	}

	string[length] = '\0';

	return string;
}

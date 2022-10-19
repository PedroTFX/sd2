#ifndef _TREE_PRIVATE_H
#define _TREE_PRIVATE_H

#include "entry.h"

struct tree_t {
	struct entry_t* node;
	struct tree_t* tree_left;
	struct tree_t* tree_right;
};

struct tree_t* get_tree(struct tree_t* tree, char* key);

int tree_get_keys_aux(struct tree_t *tree, char **key, int index);

int tree_get_values_aux(struct tree_t *tree, struct data_t **values, int index);

struct tree_t* tree_dup(struct tree_t* tree);

void print_tree(struct tree_t* tree);
#endif

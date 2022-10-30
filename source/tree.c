// Trabalho realizado por:
// Pedro Trindade 56342
// Joao Santos 56380
// Marcus Gomes 56326
#include <stdlib.h>
#include <data.h>
#include <tree.h>
#include <tree-private.h>
#include <string.h>
#include <stdio.h>

/* Função para criar uma nova árvore tree vazia.
 * Em caso de erro retorna NULL.
 */
struct tree_t* tree_create(){
	struct tree_t* tree = malloc(sizeof(struct tree_t));
	if(tree == NULL){
		tree = NULL;
		return NULL;
	}
	memset(tree, 0, sizeof(struct tree_t));
	return tree;
}


/* Função para libertar toda a memória ocupada por uma árvore.
 */
void tree_destroy(struct tree_t *tree){
	if(tree){
		if(tree->node){
			entry_destroy(tree->node);
		}
		if(tree->tree_left){
			tree_destroy(tree->tree_left);
		}
		if(tree->tree_right){
			tree_destroy(tree->tree_right);
		}
		free(tree);
		tree = NULL;
	}
}

/**
 * @brief Find the tree object from given key
 *
 * @param tree
 * @param key
 * @return struct entry_t* || NULL if not found
 */
struct tree_t* get_tree(struct tree_t* tree, char* key){
	struct data_t* data = data_create(1);
	struct entry_t* entry = entry_create(strdup(key), data);
	struct tree_t* current_tree = tree;

	while(current_tree->node != NULL){
		int comp = entry_compare(entry, current_tree->node);
		if(comp == 0){	//found
			entry_destroy(entry);
			return current_tree;
		}else if(comp == -1 && current_tree->tree_left){
			current_tree = current_tree->tree_left;
		}else if(comp == 1 && current_tree->tree_right){
			current_tree = current_tree->tree_right;
		}else{			//not found
			break;
		}
	}
	entry_destroy(entry);
	return NULL;

}

/* Função para adicionar um par chave-valor à árvore.
 * Os dados de entrada desta função deverão ser copiados, ou seja, a
 * função vai *COPIAR* a key (string) e os dados para um novo espaço de
 * memória que tem de ser reservado. Se a key já existir na árvore,
 * a função tem de substituir a entrada existente pela nova, fazendo
 * a necessária gestão da memória para armazenar os novos dados.
 * Retorna 0 (ok) ou -1 em caso de erro.
 */
int tree_put(struct tree_t *tree, char *key, struct data_t *value) {
	struct tree_t* current_tree = tree;
	struct entry_t* entry = entry_create(strdup(key), data_dup(value));

	while(current_tree->node){
		int comp = entry_compare(entry, current_tree->node);

		if(comp == -1){
			if(!current_tree->tree_left){
				current_tree->tree_left = tree_create();
			}
			current_tree = current_tree->tree_left;
		}else if(comp == 1){
			if(!current_tree->tree_right){
				current_tree->tree_right = tree_create();
			}
			current_tree = current_tree->tree_right;
		}else{				//replace
			entry_destroy(current_tree->node);		//clean bef put
			break;
		}
	}

	current_tree->node = entry_dup(entry);
	entry_destroy(entry);
	return 0;
}


/* Função para obter da árvore o valor associado à chave key.
 * A função deve devolver uma cópia dos dados que terão de ser
 * libertados no contexto da função que chamou tree_get, ou seja, a
 * função aloca memória para armazenar uma *CÓPIA* dos dados da árvore,
 * retorna o endereço desta memória com a cópia dos dados, assumindo-se
 * que esta memória será depois libertada pelo programa que chamou
 * a função. Devolve NULL em caso de erro.
 */
struct data_t* tree_get(struct tree_t *tree, char *key){
	struct tree_t* tree_st = get_tree(tree, key);
	return (tree_st) ? data_dup(tree_st->node->value) : NULL ;
}

/* Função que devolve o número de elementos contidos na árvore.
 */
int tree_size(struct tree_t *tree){
	if(tree == NULL){
		return 0;
	}
	return tree_size(tree->tree_left) + tree_size(tree->tree_right) + (tree->node != NULL);
}

//WORKS
struct tree_t* tree_dup(struct tree_t* tree){
	struct tree_t* tree_st = tree_create();

	if(tree){
		if(tree->node){
			tree_st->node = entry_dup(tree->node);
		}
		if(tree->tree_left){
			tree_st->tree_left = tree_dup(tree->tree_left);
		}
		if(tree->tree_right){
			tree_st->tree_right = tree_dup(tree->tree_right);
		}
	}
	return tree_st;
}

void tree_replace(struct tree_t* dest, struct tree_t* src){
	struct tree_t* temp = tree_dup(src);
	tree_destroy(dest);
	dest = temp;
}

struct entry_t* min(struct tree_t* tree){
	struct tree_t* temp = tree;
	while(temp->tree_left){
		temp = temp->tree_left;
	}
	return temp->node;
}

void print_tree(struct tree_t* tree){
	if(tree->tree_left){
		print_tree(tree->tree_left);
	}
	if(tree->node){
		printf("%s\n", tree->node->key);
	}

	if(tree->tree_right){
		print_tree(tree->tree_right);
	}

}


/* Função para remover um elemento da árvore, indicado pela chave key,
 * libertando toda a memória alocada na respetiva operação tree_put.
 * Retorna 0 (ok) ou -1 (key not found).
 */
int tree_del(struct tree_t *tree, char *key){

	struct tree_t* sub_tree = get_tree(tree, key);
	if(!sub_tree){
		return -1;
	}

	if(tree_size(sub_tree) == 1){ //works				//if the PD is a leaf
		entry_destroy(sub_tree->node);
        sub_tree->node = NULL;
        sub_tree = NULL;
		return 0;

	}else if(sub_tree->tree_left && sub_tree->tree_right){		//if the PD is not a leaf and has two trees duplicate the next biggest value(tree_right)
		struct entry_t* tree_min = min(sub_tree->tree_right);
		entry_replace(sub_tree->node, tree_min->key, tree_min->value);
		tree_del(sub_tree->tree_right, tree_min->key);
	}else if(sub_tree->tree_left){
		entry_replace(sub_tree->node, sub_tree->tree_left->node->key, sub_tree->tree_left->node->value);
		tree_del(sub_tree->tree_left, sub_tree->tree_left->node->key);

	}else if(sub_tree->tree_right){
		entry_replace(sub_tree->node, sub_tree->tree_right->node->key, sub_tree->tree_right->node->value);
		tree_del(sub_tree->tree_right, sub_tree->tree_right->node->key);
	}

	return 0;
}

/* Função que devolve a altura da árvore.
 */
int tree_height(struct tree_t *tree){
	if(tree == NULL){
		return 0;
	}

	int left = tree_height(tree->tree_left) + 1;
	int right = tree_height(tree->tree_right) + 1;

	return (left > right) ? left : right;
}

/* Função que devolve um array de char* com a cópia de todas as keys da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária. As keys devem vir ordenadas segundo a ordenação lexicográfica das mesmas.
 */
char **tree_get_keys(struct tree_t *tree) {
	int size = tree_size(tree) + 1;
	char **keyPtrs = malloc(size * sizeof(char *));
	if(!keyPtrs){
		return NULL;
	}

	tree_get_keys_aux(tree, keyPtrs, 0);
	keyPtrs[size - 1] = NULL;
	return keyPtrs;
}

int tree_get_keys_aux(struct tree_t *tree, char **keyPtrs, int index) {
	if (!tree || !tree->node){
		return index;
	}
	//a tree ja esta ordenada de forma lexicografica from TL to N to TR
	if(tree->tree_left){
		index = tree_get_keys_aux(tree->tree_left, keyPtrs, index);
	}

	keyPtrs[index] = strdup(tree->node->key);
		index++;
		
	if(tree->tree_right){
		index = tree_get_keys_aux(tree->tree_right, keyPtrs, index);
	}

	return index;
}

/* Função que devolve um array de void* com a cópia de todas os values da
 * árvore, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 */
void ** tree_get_values(struct tree_t *tree) {

	int size = tree_size(tree) + 1;
	struct data_t **valuePtrs = malloc(size * sizeof(struct data_t*));
  	if(!valuePtrs){
		return NULL;
	}

	tree_get_values_aux(tree, valuePtrs, 0);
	valuePtrs[size - 1] = NULL;
	return (void **) valuePtrs;
}


int tree_get_values_aux(struct tree_t *tree, struct data_t **valuePtrs, int index) {
	if (!tree || !tree->node){
		return index;
	}
	//a tree ja esta ordenada de forma lexicografica from TL to N to TR
	if(tree->tree_left){
		index = tree_get_values_aux(tree->tree_left, valuePtrs, index);
	}

	valuePtrs[index] = data_dup(tree->node->value);
		index++;
		
	if(tree->tree_right){
		index = tree_get_values_aux(tree->tree_right, valuePtrs, index);
	}
	
	return index;
}

void tree_free_keys(char **keys) {
	for (int i = 0; keys[i]; i++){
		free(keys[i]);
		keys[i] = NULL;
	}
	free(keys);
	keys = NULL;
}

/* Função que liberta toda a memória alocada por tree_get_values().
 */

void tree_free_values(void **values) {
	for (int i = 0; values[i] != NULL; i++) {
		data_destroy(values[i]);
	}
	free(values);
	values = NULL;
}

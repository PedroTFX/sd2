#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "entry.h"
#include "message-private.h"
#include "sdmessage.pb-c.h"
#include "tree.h"
#include "tree_skel-private.h"
#include "tree_skel.h"

struct tree_t* tree;
/* Inicia o skeleton da árvore.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke().
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int tree_skel_init() {
	tree = tree_create();
	if (tree == NULL) {
		return -1;
	}
	return 0;
}

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy() {
	tree_destroy(tree);
}

/* Executa uma operação na árvore (indicada pelo opcode contido em msg)
 * e utiliza a mesma estrutura message_t para devolver o resultado.
 * Retorna 0 (OK) ou -1 (erro, por exemplo, árvore nao incializada)
 */
int invoke(struct message_t* msg) {
	if (tree == NULL) {
		return -1;
	}

	if (msg->opcode == M_OPCODE_PUT) {
		invoke_put(msg);
	} else if (msg->opcode == M_OPCODE_GET) {
		invoke_get(msg);
	} else if (msg->opcode == M_OPCODE_DEL) {
		invoke_del(msg);
	} else if (msg->opcode == M_OPCODE_SIZE) {
		invoke_size(msg);
	} else if (msg->opcode == M_OPCODE_HEIGHT) {
		invoke_heigth(msg);
	} else if (msg->opcode == M_OPCODE_GETKEYS) {
		invoke_get_keys(msg);
	} else if (msg->opcode == M_OPCODE_GETVALUES) {
		invoke_get_values(msg);
	}

	return 0;
}

void invoke_put(struct message_t* msg) {
	int result = tree_put(tree, msg->entry->key, (struct data_t*)&(msg->entry->value));
	message_t__init(msg);
	msg->opcode = (result == 0) ? (MESSAGE_T__OPCODE__OP_PUT + 1) : MESSAGE_T__OPCODE__OP_ERROR;
	msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
}

void invoke_get(struct message_t* msg) {
	struct data_t* result = data_dup(tree_get(tree, msg->key));
	message_t__init(msg);
	msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
	msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
	msg->value.len = result != NULL ? result->datasize : 0;
	//msg->value.data = result != NULL ? result->data : NULL;

	data_destroy(result);
}

void invoke_del(struct message_t* msg) {
	int result = tree_del(tree, msg->key);
	free(msg->key);
	message_t__init(msg);
	msg->opcode = (result == 0) ? (MESSAGE_T__OPCODE__OP_DEL + 1) : MESSAGE_T__OPCODE__OP_ERROR;
	msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
}

void invoke_size(struct message_t* msg) {
	message_t__init(msg);
	msg->result = tree_size(tree);
	msg->opcode = (msg->result) >= 0 ? MESSAGE_T__OPCODE__OP_SIZE + 1 : MESSAGE_T__OPCODE__OP_ERROR;
	msg->c_type = (msg->result) >= 0 ? MESSAGE_T__C_TYPE__CT_RESULT : MESSAGE_T__C_TYPE__CT_NONE;
}

void invoke_heigth(struct message_t* msg) {
	message_t__init(msg);
	msg->result = tree_height(tree);
	msg->opcode = (msg->result) >= 0 ? MESSAGE_T__OPCODE__OP_HEIGHT + 1 : MESSAGE_T__OPCODE__OP_ERROR;
	msg->c_type = (msg->result) >= 0 ? MESSAGE_T__C_TYPE__CT_RESULT : MESSAGE_T__C_TYPE__CT_NONE;
}

void invoke_get_keys(struct message_t* msg) {
	char** keys = tree_get_keys(tree);
	message_t__init(msg);
	if (keys == NULL) {
		msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
		msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
		return;
	}
	msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
	msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;

	int n_keys = 0;
	while (keys[n_keys] != NULL) {
		n_keys++;
	}
	msg->n_keys = n_keys;

	msg->keys = (char**)malloc(n_keys * sizeof(char*));
	for (int i = 0; i < n_keys; i++) {
		msg->keys[i] = strdup(keys[i]);
	}
	tree_free_keys(keys);
}

void invoke_get_values(struct message_t* msg) {
	struct data_t** values = (struct data_t**)tree_get_values(tree);
	message_t__init(msg);
	if (values == NULL) {
		msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
		msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
		return;
	}
	msg->opcode = MESSAGE_T__OPCODE__OP_GETKEYS + 1;
	msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
	int i = 0;
	while (values[i] != NULL) {
		i++;
	}
	msg->n_values = i;
	msg->values = (ProtobufCBinaryData*)malloc(msg->n_values * sizeof(ProtobufCBinaryData));
	for (int j = 0; j < i; j++) {
		msg->values[j].len = values[j]->datasize;
		msg->values[j].data = malloc(msg->values[j].len);
		memcpy(msg->values[j].data, values[j]->data, msg->values[j].len);
		free(values[j]->data);
		free(values[i]);
		//printf("%s\n", msg->values[j].data);
	}
	free(values);
}

/**
msg->values

[struct ProtobufCBinaryData { int len; void* data;} -> [r][o][n][a][l][d][o][\0]]
[struct ProtobufCBinaryData { int len; void* data;} -> [r][o][n][a][l][d][o][\0]]
[struct ProtobufCBinaryData { int len = 0; void* data = NULL;}
*/

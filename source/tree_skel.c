#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	if (msg->opcode == M_OPCODE_SIZE) {
		invoke_size(msg);
	} else if (msg->opcode == M_OPCODE_HEIGHT) {
		invoke_heigth(msg);
	} else if (msg->opcode == M_OPCODE_DEL) {
		invoke_del(msg);
	} else if (msg->opcode == M_OPCODE_GET) {
		invoke_get(msg);
	} else if (msg->opcode == M_OPCODE_PUT) {
		invoke_put(msg);
	} else if (msg->opcode == M_OPCODE_GETKEYS) {
		invoke_get_keys(msg);
	} else if (msg->opcode == M_OPCODE_GETVALUES) {
		invoke_get_values(msg);
	}
	return 0;
}

void invoke_size(struct message_t* msg) {
	msg->opcode++;
	msg->c_type = M_TYPE_RESULT;
	msg->result = tree_size(tree);
}

void invoke_heigth(struct message_t* msg) {
	msg->opcode++;
	msg->c_type = M_TYPE_RESULT;
	msg->result = tree_height(tree);
}

void invoke_put(struct message_t* msg) {
	msg->opcode++;
	msg->result = tree_put(tree, msg->key, (struct data_t*)&(msg->value));
	free(msg->key);
	free(msg->value.data);
	msg->value.len = 0;
}

void invoke_del(struct message_t* msg) {
	msg->opcode++;
	msg->result = tree_del(tree, msg->key);
	free(msg->key);
}

void invoke_get(struct message_t* msg) {
	msg->opcode++;
	struct data_t* result = tree_get(tree, msg->key);
	msg->value.len = result->datasize;
	memcpy(msg->value.data, result->data, result->datasize);
}

void invoke_get_keys(struct message_t* msg) {
	msg->opcode++;
	msg->keys = tree_get_keys(tree);
}

void invoke_get_values(struct message_t* msg) {
	msg->opcode++;
	msg->values = (ProtobufCBinaryData*)tree_get_values(tree);
}

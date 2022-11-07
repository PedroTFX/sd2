#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "entry.h"
#include "message-private.h"
#include "sdmessage.pb-c.h"
#include "tree.h"
#include "tree-private.h"
#include "tree_skel-private.h"
#include "tree_skel.h"

struct tree_t* tree;
struct request_t* queue_head;
int last_assigned;
int n_threads;
struct op_proc op_procedure;
pthread_t* threads;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
/* Inicia o skeleton da árvore.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke().
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int tree_skel_init(int N) {
	n_threads = N;
	queue_head = NULL;
	last_assigned = 1;
	op_procedure.max_proc = 0;
	op_procedure.in_progress = (int*)calloc(N, sizeof(int));
	threads = (pthread_t*) malloc(N * sizeof(pthread_t));
	for(int i = 0; i < N; i++) {
		pthread_create(&threads[i], NULL, process_request, NULL);
	}
	return (tree = tree_create()) != NULL ? 0 : -1;
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
	} else if(msg->opcode == M_OPCODE_VERIFY){
		verify(msg->result);
	}

	return 0;
}

void invoke_put(struct message_t* msg) {

	// Create new request
	struct request_t* new_request = (struct request_t*)calloc(1, sizeof(struct request_t));
	if(new_request != NULL) {
		// Fulfill request
		new_request->op_n = last_assigned++;
		new_request->op = OP_PUT;
		new_request->key = strdup(msg->entry->key);
		new_request->data = (char*)malloc(msg->entry->value.len + 1);
		memcpy(new_request->data, msg->entry->value.data, msg->entry->value.len);
		new_request->data[msg->entry->value.len] = '\0';

		// Place new request in queue
		if(queue_head == NULL) {
			queue_head = new_request;
		} else {
			struct request_t* queue_tail = queue_head;
			while(queue_tail->next != NULL) {
				queue_tail = queue_tail->next;
			}
			queue_tail->next = new_request;
		}
	}

	// Free message values
	free(msg->entry->key);
	free(msg->entry->value.data);
	free(msg->entry);

	// Create message to send
	message_t__init(msg);
	msg->opcode = new_request != NULL ? MESSAGE_T__OPCODE__OP_PUT + 1 : MESSAGE_T__OPCODE__OP_ERROR;
	msg->c_type = new_request != NULL ? MESSAGE_T__C_TYPE__CT_RESULT : MESSAGE_T__C_TYPE__CT_NONE;
	if(new_request != NULL) {
		msg->result = new_request->op_n;
	}

	struct request_t* cursor = queue_head;
	while(cursor != NULL) {
		printf("Num: %d, %s, key: %s, value: %s\n", cursor->op_n, cursor->op == 0 ? "DEL" : "PUT", cursor->key, cursor->data);
		cursor = cursor->next;
	}
	printf("\n");

}

void invoke_get(struct message_t* msg) {
	struct data_t* result = tree_get(tree, msg->key);
	message_t__init(msg);
	msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
	msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
	msg->value.len = result != NULL ? result->datasize : 0;
	msg->value.data = result != NULL ? result->data : NULL;
}

void invoke_del(struct message_t* msg) {
// Create new request
	struct request_t* new_request = (struct request_t*)calloc(1, sizeof(struct request_t));
	if(new_request != NULL) {
		// Fulfill request
		new_request->op_n = last_assigned++;
		new_request->op = OP_DEL;
		new_request->key = strdup(msg->key);

		// Place new request in queue
		if(queue_head == NULL) {
			queue_head = new_request;
		} else {
			struct request_t* queue_tail = queue_head;
			while(queue_tail->next != NULL) {
				queue_tail = queue_tail->next;
			}
			queue_tail->next = new_request;
		}
	}

	// Free message values
	//free(msg->entry->key);
	free(msg->key);
	//free(msg->entry->value.data);
	//free(msg->entry);

	// Create message to send
	message_t__init(msg);
	msg->opcode = new_request != NULL ? MESSAGE_T__OPCODE__OP_DEL + 1 : MESSAGE_T__OPCODE__OP_ERROR;
	msg->c_type = new_request != NULL ? MESSAGE_T__C_TYPE__CT_RESULT : MESSAGE_T__C_TYPE__CT_NONE;
	if(new_request != NULL) {
		msg->result = new_request->op_n;
	}

	struct request_t* cursor = queue_head;
	while(cursor != NULL) {
		printf("Num: %d, %s, key: %s, value: %s\n", cursor->op_n, cursor->op == 0 ? "DEL" : "PUT", cursor->key, cursor->data);
		cursor = cursor->next;
	}
	printf("\n");
/* 	int result = tree_del(tree, msg->key);
	free(msg->key);
	message_t__init(msg);
	msg->opcode = (result == 0) ? (MESSAGE_T__OPCODE__OP_DEL + 1) : MESSAGE_T__OPCODE__OP_ERROR;
	msg->c_type = MESSAGE_T__C_TYPE__CT_NONE; */
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
	msg->values = (ProtobufCBinaryData*)malloc(i * sizeof(ProtobufCBinaryData));
	for (int j = 0; j < i; j++) {
		msg->values[j].len = values[j]->datasize;
		msg->values[j].data = malloc(msg->values[j].len);
		memcpy(msg->values[j].data, values[j]->data, msg->values[j].len);
		data_destroy(values[j]);
	}
	free(values);
}

/* Verifica se a operação identificada por op_n foi executada.
 */
int verify(int op_n){
	struct message_t* msg = (struct message_t*)malloc(sizeof(struct message_t));
	message_t__init(msg);
	msg->result = tree_height(tree);
	msg->opcode = (msg->result) >= 0 ? MESSAGE_T__OPCODE__OP_HEIGHT + 1 : MESSAGE_T__OPCODE__OP_ERROR;
	msg->c_type = (msg->result) >= 0 ? MESSAGE_T__C_TYPE__CT_RESULT : MESSAGE_T__C_TYPE__CT_NONE;
	return -50;
}

void* process_request(void *params) {
	//sleep(2);
	while(1) {
		if(queue_head != NULL) {
			//pthread_mutex_lock(&mutex1);
			printf("queue #BEFORE# processing request:\n");
			print_queue(queue_head);
			printf("done\n");
			// Process request
			if(queue_head->op == OP_PUT) {
				printf("Printing tree...\n\n");
				print_tree(tree);
				printf("Printed...\n\n");
				int size = strlen(queue_head->data) - 1;
				void* value = malloc(size);
				memcpy(value, queue_head->data, size);
				struct data_t* data = data_create2(size, value);
				int result = tree_put(tree, queue_head->key, data);
				printf("resultPut: %d\n", result);
				data_destroy(data);
			} else if(queue_head->op == OP_DEL) {
				int result = tree_del(tree, queue_head->key);
				printf("resultDel: %d\n", result);
			}
			// Update queue
			struct request_t* to_free = queue_head;
			queue_head = queue_head->next;
			free(to_free);
			printf("queue #AFTER# processing request:\n\n");
			print_queue(queue_head);
			printf("done\n\n");
			printf("Printing tree...\n\n");
			print_tree(tree);
			printf("Printed...\n\n");
			//pthread_mutex_unlock(&mutex1);
		}
	}
}

/**
msg->values
[struct ProtobufCBinaryData { int len; void* data;} -> [r][o][n][a][l][d][o][\0]]
[struct ProtobufCBinaryData { int len; void* data;} -> [r][o][n][a][l][d][o][\0]]
[struct ProtobufCBinaryData { int len = 0; void* data = NULL;}
*/

void print_queue(struct request_t* queue){
	if (queue != NULL) {
		struct request_t* printable_queue = queue;
		while (printable_queue != NULL) {
			printf("op_n:%d\n", printable_queue->op_n);
			printf("op:%d\n", printable_queue->op);
			printf("key:%s\n", printable_queue->key);
			if (printable_queue->data != NULL) {
				printf("value:%s\n", printable_queue->data);
			}
			printable_queue = printable_queue->next;
		}
	} else{
		printf("queue empty!\n");
	}
}

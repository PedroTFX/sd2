#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

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
int close_threads;
pthread_mutex_t mutex_queue = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_op_proc = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_tree = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_cond = PTHREAD_COND_INITIALIZER;
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
	close_threads = 0;
	for(int i = 0; i < N; i++) {
		pthread_create(&threads[i], NULL, process_request, NULL);
	}
	return (tree = tree_create()) != NULL ? 0 : -1;
}

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy() {
	close_threads = 1;
 	for (int i = 0; i < n_threads; i++) {
		pthread_join(threads[i], NULL);
	}
	printf("Threads returned successfully\n");
	//pthread_mutex_destroy(&mutex1);
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
	while(close_threads == 0) {

		// Sleep until there are requests in the queue
		//pthread_cond_wait(&condition_cond, NULL);

		//printf("Running thread...\n");
		struct request_t* request;
		pthread_mutex_lock(&mutex_queue); // Lock access to queue
		if(queue_head == NULL) { // Check if there are requests in the queue
			pthread_mutex_unlock(&mutex_queue); // Unlock access to queue
			continue;
		}
		request = queue_head;
		queue_head = queue_head->next;
		pthread_mutex_unlock(&mutex_queue); // Unlock access to queue

		// Execute request
		if(request->op == OP_PUT) {
			printf("\nThread will execute put operation.\n");

			int size = strlen(request->data);
			void* value = malloc(size);
			memcpy(value, request->data, size);
			struct data_t* data = data_create2(size, value);

			pthread_mutex_lock(&mutex_tree); // Lock access to tree
			print_tree(tree); // Print tree
			int result = tree_put(tree, request->key, data);
			print_tree(tree); // Print tree
			pthread_mutex_unlock(&mutex_tree); // Unlock access to tree

			// Free request and data
			data_destroy(data);

			// Check for error
			if(result == -1) {
				printf("Error processing put request!\n");
				continue;
			}
			printf("PUT successful!\n");
		} else if(request->op == OP_DEL) {
			printf("\nThread will execute delete operation.\n");

			// Execute delete
			pthread_mutex_lock(&mutex_tree); // Lock access to tree
			print_tree(tree); // Print tree
			int result = tree_del(tree, request->key);
			print_tree(tree); // Print tree
			pthread_mutex_unlock(&mutex_tree); // Unlock access to tree

			// Check for error
			if(result == -1) {
				printf("Error processing delete request!\n");
				continue;
			}
			printf("DELETE successful!\n");
		}

		// Free request
		request_destroy(request);
	}
	printf("Closing thread.\n");
	return NULL;
}

/**
msg->values
[struct ProtobufCBinaryData { int len; void* data;} -> [r][o][n][a][l][d][o][\0]]
[struct ProtobufCBinaryData { int len; void* data;} -> [r][o][n][a][l][d][o][\0]]
[struct ProtobufCBinaryData { int len = 0; void* data = NULL;}
*/

void print_queue(struct request_t* queue) {
	if(queue == NULL) {
		printf("QUEUE: empty\n");
		return;
	}

	struct request_t* cursor = queue_head;
	while(cursor != NULL) {
		printf("QUEUE: Num: %d, %s, key: %s, value: %s\n", cursor->op_n, cursor->op == 0 ? "DEL" : "PUT", cursor->key, cursor->data);
		cursor = cursor->next;
	}
}

void request_destroy(struct request_t* request){
	free(request->key);
	free(request->data);
	free(request);
}

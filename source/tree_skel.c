#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "entry.h"
#include "message-private.h"
#include "sdmessage.pb-c.h"
#include "tree-private.h"
#include "tree.h"
#include "tree_skel-private.h"
#include "tree_skel.h"

struct tree_t* tree;
struct request_t* queue_head;
int last_assigned;
int n_threads;
struct op_proc op_procedure;
pthread_t* threads;
int close_threads;
int* thread_ids;
pthread_mutex_t mutex_queue = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_op_proc = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_tree = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
/* Inicia o skeleton da árvore.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke().
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int tree_skel_init(int N) {
	pthread_cond_init(&queue_not_empty, NULL);
	n_threads = N;
	queue_head = NULL;
	last_assigned = 1;
	op_procedure.max_proc = 0;
	op_procedure.in_progress = (int*)calloc(N, sizeof(int));
	threads = (pthread_t*)malloc(N * sizeof(pthread_t));
	close_threads = 0;
	thread_ids = (int*)malloc(N * sizeof(int));
	tree = tree_create();
	for (int i = 0; i < N; i++) {
		thread_ids[i] = i;
		pthread_create(&threads[i], NULL, process_request, (void*)&(thread_ids[i]));
	}
	return tree != NULL ? 0 : -1;
}

/* Liberta toda a memória e recursos alocados pela função tree_skel_init.
 */
void tree_skel_destroy() {
	close_threads = 1;
	pthread_cond_broadcast(&queue_not_empty);
	for (int i = 0; i < n_threads; i++) {
		pthread_join(threads[i], NULL);
	}
	printf("Threads returned successfully\n");
	printf("max_proc:%d\n", op_procedure.max_proc);
	print_queue(-1, queue_head);
	print_tree_id(-1, tree);
	free(op_procedure.in_progress);
	free(thread_ids);
	free(threads);
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
	} else if (msg->opcode == M_OPCODE_VERIFY) {
		invoke_verify(msg);
	}
	return 0;
}

void invoke_put(struct message_t* msg) {
	// Create new request
	struct request_t* new_request = (struct request_t*)calloc(1, sizeof(struct request_t));
	if (new_request != NULL) {
		// Fulfill request
		new_request->op_n = last_assigned++;
		new_request->op = OP_PUT;
		new_request->key = strdup(msg->entry->key);
		new_request->data = (char*)malloc(msg->entry->value.len + 1);
		memcpy(new_request->data, msg->entry->value.data, msg->entry->value.len);
		new_request->data[msg->entry->value.len] = '\0';
		// Place new request in queue
		queue_add_task(new_request);
	}
	// Free message values
	free(msg->entry->key);
	free(msg->entry->value.data);
	free(msg->entry);

	// Create message to send
	message_t__init(msg);
	msg->opcode = new_request != NULL ? MESSAGE_T__OPCODE__OP_PUT + 1 : MESSAGE_T__OPCODE__OP_ERROR;
	msg->c_type = new_request != NULL ? MESSAGE_T__C_TYPE__CT_RESULT : MESSAGE_T__C_TYPE__CT_NONE;
	if (new_request != NULL) {
		msg->result = new_request->op_n;
	}

	pthread_cond_signal(&queue_not_empty);
}

void invoke_get(struct message_t* msg) {
	pthread_mutex_lock(&mutex_tree);
	struct data_t* result = tree_get(tree, msg->key);
	pthread_mutex_unlock(&mutex_tree);
	message_t__init(msg);
	msg->opcode = MESSAGE_T__OPCODE__OP_GET + 1;
	msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
	msg->value.len = result != NULL ? result->datasize : 0;
	msg->value.data = result != NULL ? result->data : NULL;
}

void invoke_del(struct message_t* msg) {
	// Create new request
	struct request_t* new_request = (struct request_t*)calloc(1, sizeof(struct request_t));
	if (new_request != NULL) {
		// Fulfill request
		new_request->op_n = last_assigned++;
		new_request->op = OP_DEL;
		new_request->key = strdup(msg->key);

		// Place new request in queue
		queue_add_task(new_request);
	}

	// Free message values
	free(msg->key);

	// Create message to send
	message_t__init(msg);
	msg->opcode = new_request != NULL ? MESSAGE_T__OPCODE__OP_DEL + 1 : MESSAGE_T__OPCODE__OP_ERROR;
	msg->c_type = new_request != NULL ? MESSAGE_T__C_TYPE__CT_RESULT : MESSAGE_T__C_TYPE__CT_NONE;
	if (new_request != NULL) {
		msg->result = new_request->op_n;
	}

	pthread_cond_signal(&queue_not_empty);
}

void invoke_size(struct message_t* msg) {
	message_t__init(msg);
	pthread_mutex_lock(&mutex_tree);
	msg->result = tree_size(tree);
	pthread_mutex_unlock(&mutex_tree);
	msg->opcode = (msg->result) >= 0 ? MESSAGE_T__OPCODE__OP_SIZE + 1 : MESSAGE_T__OPCODE__OP_ERROR;
	msg->c_type = (msg->result) >= 0 ? MESSAGE_T__C_TYPE__CT_RESULT : MESSAGE_T__C_TYPE__CT_NONE;
}

void invoke_heigth(struct message_t* msg) {
	message_t__init(msg);
	pthread_mutex_lock(&mutex_tree);
	msg->result = tree_height(tree);
	pthread_mutex_unlock(&mutex_tree);
	msg->opcode = (msg->result) >= 0 ? MESSAGE_T__OPCODE__OP_HEIGHT + 1 : MESSAGE_T__OPCODE__OP_ERROR;
	msg->c_type = (msg->result) >= 0 ? MESSAGE_T__C_TYPE__CT_RESULT : MESSAGE_T__C_TYPE__CT_NONE;
}

void invoke_get_keys(struct message_t* msg) {
	pthread_mutex_lock(&mutex_tree);
	char** keys = tree_get_keys(tree);
	pthread_mutex_unlock(&mutex_tree);
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
	pthread_mutex_lock(&mutex_tree);
	struct data_t** values = (struct data_t**)tree_get_values(tree);
	pthread_mutex_unlock(&mutex_tree);
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

void invoke_verify(struct message_t* msg) {
	msg->result = verify(msg->result);
	msg->opcode = (msg->result) >= 0 ? MESSAGE_T__OPCODE__OP_VERIFY + 1 : MESSAGE_T__OPCODE__OP_ERROR;
	msg->c_type = (msg->result) >= 0 ? MESSAGE_T__C_TYPE__CT_RESULT : MESSAGE_T__C_TYPE__CT_NONE;
}

/* Verifica se a operação identificada por op_n foi executada.
 */
int verify(int op_n) {
	pthread_mutex_lock(&mutex_op_proc);
	int max_proc = op_procedure.max_proc;
	pthread_mutex_unlock(&mutex_op_proc);

	// If operation number is higher than the maximum executed until now
	if (op_n > max_proc) {
		return 0;					// For sure, it hasn't been executed yet
	} else if (op_n == max_proc) {	// If it's equal
		return 1;					// Then, this was precisely the one that was executed
	}

	// Otherwise, if it's still in the queue, then it hasn't
	pthread_mutex_lock(&mutex_queue);
	struct request_t* request = queue_head;
	while (request != NULL) {
		if (request->op_n == op_n) {
			pthread_mutex_unlock(&mutex_queue);
			return 0;
		}
		request = request->next;
	}
	pthread_mutex_unlock(&mutex_queue);

	// Otherwise, if it's still in progress, then it hasn't
	pthread_mutex_lock(&mutex_op_proc);
	for (int i = 0; i < n_threads; i++) {
		if (op_procedure.in_progress[i] == op_n) {
			pthread_mutex_unlock(&mutex_op_proc);
			return 0;
		}
	}
	pthread_mutex_unlock(&mutex_op_proc);
	// Otherwise, it has
	return 1;
}

void* process_request(void* params) {
	int id = *((int*)params);
	// int num = (int)*params;
	printf("Thread ID: %d\n", id);
	struct request_t* request;
	while ((request = queue_get_task(id)) != NULL) {
		pthread_mutex_lock(&mutex_op_proc);
		// print_op_proc(id, &op_procedure);
		op_procedure.in_progress[id] = request->op_n;
		// print_op_proc(id, &op_procedure);
		pthread_mutex_unlock(&mutex_op_proc);

		// Execute request
		if (request->op == OP_PUT) {
			printf("\n%d: Thread will execute put operation.\n", id);

			int size = strlen(request->data);
			void* value = malloc(size);
			memcpy(value, request->data, size);
			struct data_t* data = data_create2(size, value);

			pthread_mutex_lock(&mutex_tree);  // Lock access to tree
			// print_tree_id__(id, tree); // Print tree
			// print_tree(tree); // Print tree
			int result = tree_put(tree, request->key, data);
			// print_tree_id(id, tree); // Print tree
			// print_tree(tree); // Print tree
			pthread_mutex_unlock(&mutex_tree);	// Unlock access to tree


			pthread_mutex_lock(&mutex_op_proc);
			if (request->op_n > op_procedure.max_proc) {
				op_procedure.max_proc = request->op_n;
			}
			op_procedure.in_progress[id] = request->op_n;
			// Free request
			request_destroy(request);
			pthread_mutex_unlock(&mutex_op_proc);
			// Free request and data
			data_destroy(data);

			// Check for error
			if (result == -1) {
				printf("Error processing put request!\n");
				continue;
			}
			printf("%d: PUT successful!\n", id);
		} else if (request->op == OP_DEL) {
			printf("\n%d: Thread will execute delete operation.\n", id);

			// Execute delete
			pthread_mutex_lock(&mutex_tree);  // Lock access to tree
			// print_tree_id(id, tree); // Print tree
			// print_tree(tree); // Print tree
			int result = tree_del(tree, request->key);
			// print_tree_id(id, tree); // Print tree
			// print_tree(tree); // Print tree
			pthread_mutex_unlock(&mutex_tree);	// Unlock access to tree
			pthread_mutex_lock(&mutex_op_proc);
			// print_op_proc(id, &op_procedure);
			if (request->op_n > op_procedure.max_proc) {
				op_procedure.max_proc = request->op_n;
			}
			op_procedure.in_progress[id] = request->op_n;
			//print_op_proc(id, &op_procedure);
			// Free request
			request_destroy(request);
			pthread_mutex_unlock(&mutex_op_proc);

			// Check for error
			if (result == -1) {
				printf("Error processing delete request!\n");
				continue;
			}
			printf("%d: DELETE successful!\n", id);
		}
	}
	printf("Closing thread %d.\n", id);
	return NULL;
}
void print_queue(int id, struct request_t* queue) {
	if (queue == NULL) {
		printf("%d: QUEUE: empty\n", id);
		return;
	}

	struct request_t* cursor = queue_head;
	while (cursor != NULL) {
		printf("%d: QUEUE: Num: %d, %s, key: %s, value: %s\n", id, cursor->op_n, cursor->op == 0 ? "DEL" : "PUT", cursor->key, cursor->data);
		cursor = cursor->next;
	}
}

void request_destroy(struct request_t* request) {
	free(request->key);
	free(request->data);
	free(request);
}

void print_op_proc(int id, struct op_proc* op_procedure) {
	if (op_procedure != NULL) {
		printf("%d: max_proc: %d\n", id, op_procedure->max_proc);
	}
	if (op_procedure->in_progress != NULL) {
		for (int i = 0; i < n_threads; i++) {
			printf("%d, Thread:  %d, op: %d\n", id, i, op_procedure->in_progress[i]);
		}
	}
}

void queue_add_task(struct request_t* task) {
	pthread_mutex_lock(&mutex_queue);
	if (queue_head == NULL) { /* Adiciona na cabeça da fila */
		queue_head = task;
		task->next = NULL;
	} else { /* Adiciona no fim da fila */
		struct request_t* tptr = queue_head;
		while (tptr->next != NULL)
			tptr = tptr->next;
		tptr->next = task;
		task->next = NULL;
	}
	pthread_cond_signal(&queue_not_empty); /* Avisa um bloqueado nessa condição */
	pthread_mutex_unlock(&mutex_queue);
}

struct request_t* queue_get_task(int id) {
	pthread_mutex_lock(&mutex_queue);
	while (queue_head == NULL) {
		printf("Sleeping thread %d.\n", id);
		pthread_cond_wait(&queue_not_empty, &mutex_queue); /* Espera haver algo */
		printf("Waking up thread %d.\n", id);
		if (close_threads == 1) {
			pthread_mutex_unlock(&mutex_queue);
			return NULL;
		}
	}
	struct request_t* task = queue_head;
	queue_head = task->next;
	pthread_mutex_unlock(&mutex_queue);
	return task;
}

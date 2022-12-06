#ifndef _TREE_SKEL_PRIVATE_H
#define _TREE_SKEL_PRIVATE_H
#include <zookeeper.h>

#define OP_DEL 0
#define OP_PUT 1

struct request_t {
	int op_n;	 // o número da operação
	int op;		 // a operação a executar. op=0 se for um delete, op=1 se for um put
	char* key;	 // a chave a remover ou adicionar
	char* data;	 // os dados a adicionar em caso de put, ou NULL em caso de delete
	struct request_t* next;
};

struct rtree_t* next_rtree;

struct op_proc {
	int max_proc;	   // Maior número da operação já executada
	int* in_progress;  // Array de numeros de operações em execução por cada thread
};

void select_next_server(zoo_string* children_list, char* root_path, zhandle_t* zh/* zoo_string* children_list, char* root_path */);

void invoke_size(struct message_t*);

void invoke_heigth(struct message_t*);

void invoke_put(struct message_t*);

void invoke_del(struct message_t*);

void invoke_get(struct message_t*);

void invoke_get_keys(struct message_t*);

void invoke_get_values(struct message_t*);

void print_queue(int id, struct request_t*);

void request_destroy(struct request_t*);

void invoke_verify(struct message_t* msg);

void print_op_proc(int id, struct op_proc* op_procedure);

void queue_add_task(struct request_t* task);

struct request_t* queue_get_task(int id);

#endif

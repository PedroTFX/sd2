#define OP_DEL 0
#define OP_PUT 1

struct request_t {
	int op_n;	 // o número da operação
	int op;		 // a operação a executar. op=0 se for um delete, op=1 se for um put
	char* key;	 // a chave a remover ou adicionar
	char* data;	 // os dados a adicionar em caso de put, ou NULL em caso de delete
	struct request_t* next;
};

struct op_proc {
	int max_proc;	   // Maior número da operação já executada
	int* in_progress;  // Array de numeros de operações em execução por cada thread
};

void invoke_size(struct message_t*);

void invoke_heigth(struct message_t*);

void invoke_put(struct message_t*);

void invoke_del(struct message_t*);

void invoke_get(struct message_t*);

void invoke_get_keys(struct message_t*);

void invoke_get_values(struct message_t*);

void print_queue(struct request_t*);

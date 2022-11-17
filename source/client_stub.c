#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client_stub-private.h"
#include "client_stub.h"
#include "message-private.h"
#include "network_client.h"

/* Função para estabelecer uma associação entre o cliente e o servidor,
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna NULL em caso de erro.
 */
struct rtree_t* rtree_connect(const char* address_port) {
	char* adrsport = (char*)malloc(strlen(address_port) + 1);
	strcpy(adrsport, address_port);
	char* hostname = strdup(strtok(adrsport, ":"));
	char* port = strdup(strtok(NULL, ":"));
	struct rtree_t* rtree = (struct rtree_t*)malloc(sizeof(struct rtree_t));
	rtree->address = hostname;
	rtree->port = port;
	rtree->socket_id = -1;
	free(adrsport);
	return network_connect(rtree) == -1 ? NULL : rtree;
}

/* Termina a associação entre o cliente e o servidor, fechando a
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtree_disconnect(struct rtree_t* rtree) {
	if (network_close(rtree) == -1) {
		printf("Error closing connection.\n");
	}
	free(rtree->address);
	free(rtree->port);
	free(rtree);
	printf("Closing\n");
	return 0;
}

/* Função para adicionar um elemento na árvore.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Devolve 0 (ok, em adição/substituição) ou -1 (problemas).
 */
int rtree_put(struct rtree_t* rtree, struct entry_t* entry) {
	struct message_t* request = (struct message_t*)malloc(sizeof(struct message_t));
	message_t__init(request);
	request->opcode = MESSAGE_T__OPCODE__OP_PUT;
	request->c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
	request->entry = (MessageT__Entry*)malloc(sizeof(MessageT__Entry));
	message_t__entry__init(request->entry);
	request->entry->key = entry->key;
	request->entry->value.len = entry->value->datasize;
	request->entry->value.data = entry->value->data;

	struct message_t* response = network_send_receive(rtree, request);
	if(response == NULL) {
		return -1;
	}
	free(request->entry);
	free(request);
	int result = response->opcode != MESSAGE_T__OPCODE__OP_ERROR ? response->result : -1;
	message_t__free_unpacked(response, NULL);
	return result;
}

/* Função para obter um elemento da árvore.
 * Em caso de erro, devolve NULL.
 */
struct data_t* rtree_get(struct rtree_t* rtree, char* key) {
	struct message_t* request = (struct message_t*)malloc(sizeof(struct message_t));
	message_t__init(request);
	request->opcode = MESSAGE_T__OPCODE__OP_GET;
	request->c_type = MESSAGE_T__C_TYPE__CT_KEY;
	request->key = key;
	struct message_t* response = network_send_receive(rtree, request);
	free(request);
	if (response->opcode != (MESSAGE_T__OPCODE__OP_GET + 1)) {
		return NULL;
	}
	struct data_t* data = (struct data_t*)malloc(sizeof(struct data_t));
	data->datasize = response->value.len;
	data->data = malloc(response->value.len);
	memcpy(data->data, response->value.data, response->value.len);
	message_t__free_unpacked(response, NULL);
	return data;
}

/* Função para remover um elemento da árvore. Vai libertar
 * toda a memoria alocada na respetiva operação rtree_put().
 * Devolve: 0 (ok), -1 (key not found ou problemas).
 */
int rtree_del(struct rtree_t* rtree, char* key) {
	struct message_t* request = (struct message_t*)malloc(sizeof(struct message_t));
	message_t__init(request);
	request->opcode = MESSAGE_T__OPCODE__OP_DEL;
	request->c_type = MESSAGE_T__C_TYPE__CT_KEY;
	request->key = key;
	struct message_t* response = network_send_receive(rtree, request);
	free(request);
	int result = response->opcode != MESSAGE_T__OPCODE__OP_ERROR ? response->result : -1;
	message_t__free_unpacked(response, NULL);
	return result;
}

/* Devolve o número de elementos contidos na árvore.
 */
int rtree_size(struct rtree_t* rtree) {
	struct message_t* request = (struct message_t*)malloc(sizeof(struct message_t));
	message_t__init(request);
	request->opcode = MESSAGE_T__OPCODE__OP_SIZE;
	request->c_type = MESSAGE_T__C_TYPE__CT_NONE;
	struct message_t* response = network_send_receive(rtree, request);
	free(request);
	int result = response->opcode != MESSAGE_T__OPCODE__OP_ERROR ? response->result : -1;
	message_t__free_unpacked(response, NULL);
	return result;
}

/* Função que devolve a altura da árvore.
 */
int rtree_height(struct rtree_t* rtree) {
	struct message_t* request = (struct message_t*)malloc(sizeof(struct message_t));
	message_t__init(request);
	request->opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
	request->c_type = MESSAGE_T__C_TYPE__CT_NONE;
	struct message_t* response = network_send_receive(rtree, request);
	free(request);
	int result = response->opcode != MESSAGE_T__OPCODE__OP_ERROR ? response->result : -1;
	message_t__free_unpacked(response, NULL);
	return result;
}

/* Devolve um array de char* com a cópia de todas as keys da árvore,
 * colocando um último elemento a NULL.
 */
char** rtree_get_keys(struct rtree_t* rtree) {
	struct message_t* request = (struct message_t*)malloc(sizeof(struct message_t));
	message_t__init(request);
	request->opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
	request->c_type = MESSAGE_T__C_TYPE__CT_NONE;
	struct message_t* response = network_send_receive(rtree, request);
	free(request);

	if(response == NULL) {
		return NULL;
	}

	if (response->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
		message_t__free_unpacked(response, NULL);
		return NULL;
	}

	char** keys = (char**)malloc(sizeof(char*) * (response->n_keys + 1));
	int i;
	for (i = 0; i < response->n_keys; i++) {
		keys[i] = strdup(response->keys[i]);
	}
	keys[i] = NULL;
	message_t__free_unpacked(response, NULL);
	return keys;
}

/* Devolve um array de void* com a cópia de todas os values da árvore,
 * colocando um último elemento a NULL.
 */
void** rtree_get_values(struct rtree_t* rtree) {
	struct message_t* request = (struct message_t*)malloc(sizeof(struct message_t));
	message_t__init(request);
	request->opcode = MESSAGE_T__OPCODE__OP_GETVALUES;
	request->c_type = MESSAGE_T__C_TYPE__CT_NONE;
	struct message_t* response = network_send_receive(rtree, request);
	free(request);
	if (response->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
		message_t__free_unpacked(response, NULL);
		return NULL;
	}

	struct data_t** values = (struct data_t**)malloc(sizeof(struct data_t*) * (response->n_values + 1));
	int i;
	for (i = 0; i < response->n_values; i++) {
		values[i] = (struct data_t*)malloc(sizeof(struct data_t));
		values[i]->datasize = response->values[i].len;
		values[i]->data = malloc(response->values[i].len);
		memcpy(values[i]->data, response->values[i].data, response->values[i].len);
	}
	values[i] = NULL;
	message_t__free_unpacked(response, NULL);
	return (void**)values;
}

/* Verifica se a operação identificada por op_n foi executada.
 */
int rtree_verify(struct rtree_t *rtree, int op_n){
	struct message_t* request = (struct message_t*)malloc(sizeof(struct message_t));
	message_t__init(request);
	request->opcode = MESSAGE_T__OPCODE__OP_VERIFY;
	request->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
	request->result = op_n;
	struct message_t* response = network_send_receive(rtree, request);
	free(request);
	if (response->opcode == MESSAGE_T__OPCODE__OP_ERROR) {
		message_t__free_unpacked(response, NULL);
		return -1;
	}
	int result = response->result;
	message_t__free_unpacked(response, NULL);
	return result;
}

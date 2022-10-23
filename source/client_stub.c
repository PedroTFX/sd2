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
	char* hostname = strtok(adrsport, ":");
	char* port = strtok(NULL, ":");
	struct rtree_t* rtree = (struct rtree_t*)malloc(sizeof(struct rtree_t));
	rtree->address = hostname;
	rtree->port = port;
	rtree->socket_id = -1;
	// rtree->root = tree_create();
	return network_connect(rtree) == -1 ? NULL : rtree;
}

/* Termina a associação entre o cliente e o servidor, fechando a
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtree_disconnect(struct rtree_t* rtree) {
	network_close(rtree);
	//free(rtree->address);
	//free(rtree->port);
	// tree_destroy(rtree->root);
	//free(rtree);
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
	return (response->opcode == MESSAGE_T__OPCODE__OP_PUT + 1) ? 0 : -1;
}

/* Função para obter um elemento da árvore.
 * Em caso de erro, devolve NULL.
 */
struct data_t* rtree_get(struct rtree_t* rtree, char* key) {
	return NULL;
}

/* Função para remover um elemento da árvore. Vai libertar
 * toda a memoria alocada na respetiva operação rtree_put().
 * Devolve: 0 (ok), -1 (key not found ou problemas).
 */
int rtree_del(struct rtree_t* rtree, char* key) {
	return 0;
}

/* Devolve o número de elementos contidos na árvore.
 */
int rtree_size(struct rtree_t* rtree) {
	return 0;
}

/* Função que devolve a altura da árvore.
 */
int rtree_height(struct rtree_t* rtree) {
	return 0;
}

/* Devolve um array de char* com a cópia de todas as keys da árvore,
 * colocando um último elemento a NULL.
 */
char** rtree_get_keys(struct rtree_t* rtree) {
	return NULL;
}

/* Devolve um array de void* com a cópia de todas os values da árvore,
 * colocando um último elemento a NULL.
 */
void** rtree_get_values(struct rtree_t* rtree) {
	return NULL;
}

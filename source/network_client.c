#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "client_stub-private.h"
#include "sdmessage.pb-c.h"
#include "network_client-private.h"
#include "message-private.h"
#include "tree.h"
/* Esta função deve:
 * - Obter o endereço do servidor (struct sockaddr_in) a base da
 *   informação guardada na estrutura rtree;
 * - Estabelecer a ligação com o servidor;
 * - Guardar toda a informação necessária (e.g., descritor do socket)
 *   na estrutura rtree;
 * - Retornar 0 (OK) ou -1 (erro).
 */
int network_connect(struct rtree_t *rtree){
    //socket info
    struct sockaddr_in server_info ={0};
    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = rtree->address;
    server_info.sin_port = rtree->port;

    socklen_t server_info_len = sizeof(server_info);

    //socket
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd < 0){
        perror("socket");
        return -1;
    }
	rtree->socket_id = sfd;
    //connect
	int con = connect(sfd, (struct sockaddr*)&server_info, server_info_len);
    if(con == -1){
        perror("connect");
        return -1;
    }

    return 0;
}

/* Esta função deve:
 * - Obter o descritor da ligação (socket) da estrutura rtree_t;
 * - Serializar a mensagem contida em msg;
 * - Enviar a mensagem serializada para o servidor;
 * - Esperar a resposta do servidor;
 * - De-serializar a mensagem de resposta;
 * - Retornar a mensagem de-serializada ou NULL em caso de erro.
 */
struct message_t *network_send_receive(struct rtree_t * rtree, struct message_t *msg){
	int sfd = rtree->socket_id;
	uint8_t buffer[BUFFER_MAX_SIZE];
	int buffer_size = message_t__pack(msg, buffer);
	//message_t__free_unpacked(msg, NULL);
	if (send(sfd, buffer, buffer_size, 0) == -1) {
		perror("could not send data\n");
	}
	int size = read(sfd, buffer, BUFFER_MAX_SIZE);
	struct message_t* msg2 = message_t__unpack(NULL, size, buffer);
	return msg2;
}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtree_t * rtree){
	return close(rtree->socket_id);
}

int main(int argc, char const *argv[]) {
	struct rtree_t server_tree;
	server_tree.address = htonl(0x7f000001); // 127.0.0.1
	//server_tree.address = inet_address("localhost");
	server_tree.port = htons(1337);
	server_tree.socket_id = -1;
	server_tree.root = tree_create();

	network_connect(&server_tree);
	struct message_t* request;
	message_t__init(request);
	request->opcode = MESSAGE_T__OPCODE__OP_HEIGHT;
	request->c_type = MESSAGE_T__C_TYPE__CT_NONE;
	struct message_t* response = network_send_receive(&server_tree, request);
	printf("OPCODE: %d\n", response->opcode);
	printf("CTYPE: %d\n", response->c_type);
	printf("HEIGHT: %d\n", response->result);
	network_close(&server_tree);
	return 0;
}

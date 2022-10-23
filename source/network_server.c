#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <tree.h>
#include <unistd.h>

#include "tree_skel.h"
#include "network_server.h"
#include "message-private.h"

int sfd;

/* Função para preparar uma socket de receção de pedidos de ligação
 * num determinado porto.
 * Retornar descritor do socket (OK) ou -1 (erro).
 */
int network_server_init(short port) {
	// socket info struct
	struct sockaddr_in server_info = {0};
	server_info.sin_family = AF_INET;
	server_info.sin_port = htons(port);
	socklen_t server_info_len = sizeof(struct sockaddr_in);

	// socket
	//  +++int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return -1;
	}
	printf("socket created\n");
	int option_value = 1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int));
	// bind to addr
	if (bind(sfd, (struct sockaddr*)&server_info, server_info_len) < 0) {
		perror("bind");
		return -1;
	}
	printf("socket bound\n");

	// listen for connections
	if (listen(sfd, 0) < 0) {  // right declaration
		perror("listen failed");
		return -1;
	}
	printf("listen\n");

	return sfd;
}

/* Esta função deve:
 * - Aceitar uma conexão de um cliente;
 * - Receber uma mensagem usando a função network_receive;
 * - Entregar a mensagem de-serializada ao skeleton para ser processada;
 * - Esperar a resposta do skeleton;
 * - Enviar a resposta ao cliente usando a função network_send.
 */
int network_main_loop(int listening_socket) {
	// accept
	struct sockaddr client_info = {0};
	socklen_t client_info_len = sizeof(client_info);

	// int sfd = accept(listening_socket, &client_info, &client_info_len);
	if ((sfd = accept(listening_socket, &client_info, &client_info_len)) < 0) {
		close(sfd);
		perror("accept failed");
		return -1;
	}

	printf("New client connected\n");
	while (1) {
		struct message_t* msg = network_receive(sfd);
		invoke(msg);
		network_send(sfd, msg);
	}
	// free msg
	close(listening_socket);  // we might not need this one anymore, for further testing

	return sfd;
}

/* Esta função deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura message_t.
 */
struct message_t* network_receive(int client_socket) {
	uint8_t buff[1000];
	int size = read(sfd, buff, 1000);
	struct message_t* msg = message_t__unpack(NULL, size, buff);
	return msg;
}

/* Esta função deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Libertar a memória ocupada por esta mensagem;
 * - Enviar a mensagem serializada, através do client_socket.
 */
int network_send(int client_socket, struct message_t* msg) {
	uint8_t buffer[BUFFER_MAX_SIZE];
	int buffer_size = message_t__pack(msg, buffer);

	ssize_t sent = send(client_socket, buffer, buffer_size, 0);
	//close(client_socket);
	return sent;
}

/* A função network_server_close() liberta os recursos alocados por
 * network_server_init().
 */
int network_server_close() {
	close(sfd);
	return 0;
}

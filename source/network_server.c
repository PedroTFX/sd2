#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <tree.h>
#include <unistd.h>

#include "message-private.h"
#include "network_server-private.h"
#include "network_server.h"
#include "tree_skel-private.h"
#include "tree_skel.h"
#include "util.h"

int listening_socket;

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

	// Ignore SIGPIPE signal so server doesn't crash if socket closes unexpectedly
	struct sigaction new_actn;
	new_actn.sa_handler = SIG_IGN;
	sigemptyset(&new_actn.sa_mask);
	new_actn.sa_flags = 0;
	sigaction(SIGPIPE, &new_actn, NULL);

	// socket
	//  +++int listening_socket = socket(AF_INET, SOCK_STREAM, 0);
	if ((listening_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	int option_value = 1;
	setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int));
	// bind to addr
	if (bind(listening_socket, (struct sockaddr*)&server_info, server_info_len) < 0) {
		perror("bind");
		return -1;
	}

	// listen for connections
	if (listen(listening_socket, 0) < 0) {	// right declaration
		perror("listen failed");
		return -1;
	}
	printf("Listening on port %hu...\n", port);

	return listening_socket;
}

/* Esta função deve:
 * - Aceitar uma conexão de um cliente;
 * - Receber uma mensagem usando a função network_receive;
 * - Entregar a mensagem de-serializada ao skeleton para ser processada;
 * - Esperar a resposta do skeleton;
 * - Enviar a resposta ao cliente usando a função network_send.
 */

// FUNCAO CORRETA
/* int network_main_loop(int listening_socket) {
	struct sockaddr client_info = {0};
	socklen_t client_info_len = sizeof(client_info);
	int client_socket;
	while ((client_socket = accept(listening_socket, &client_info, &client_info_len)) > 0) {
		printf("Client connected\n");

		struct message_t* msg;
		while ((msg = network_receive(client_socket)) != NULL) {
			invoke(msg);
			network_send(client_socket, msg);
		}
		close(client_socket);
		printf("Client disconnected\n");
	}
	return 0;
} */

/* Esboço do algoritmo a ser implementado na função network_main_loop */
// adiciona listening_socket a desc_set. /* desc_set corresponde a um conjunto de file descriptors */
/*
while (poll(desc_set) >= 0) {				   // Espera por dados nos sockets abertos
	if (listening_socket tem dados para ler) { //Verifica se tem novo pedido de conexão
		connsockfd = accept(listening_socket);
		adiciona connsockfd a desc_set
	}
	for (all socket s em desc_set, excluindo listening_socket) { // Verifica restantes sockets
		if (s tem dados para ler) {
			message = network_receive(s);
			if (message é NULL) { // Sinal de que a conexão foi fechada pelo cliente
				close(s);
				remove s de desc_set
			} else {
				invoke(message);	   // Executa pedido contido em message
				network_send(message); // Envia resposta contida em message
			}
		}
		if (s com erro ou POLLHUP) {
			close(s);
			remove s de desc_set
		}
	}
}
*/

int network_main_loop(int listening_socket) {
	int num_fds = 1;  // Initialize connections array with the first position for the listening socket
	struct pollfd* connections = (struct pollfd*) malloc(num_fds * sizeof(struct pollfd));
	if (connections == NULL) {
		perror("Could not initialize connections array.\n");
		return -1;
	}
	connections->events = POLLIN;		 // There is data to read...
	connections->fd = listening_socket;	 // ...on the welcoming socket
	while ((poll(connections, num_fds, TIMEOUT)) > 0) {  // kfds == 0 significa timeout sem eventos
		// If there are new clients wanting to connect, let's accept their connection (indice 0 implicito)
		struct sockaddr client_info = {0};
		socklen_t client_info_len = sizeof(client_info);
		int new_client_fd;
		if ((connections->revents & POLLIN) && (new_client_fd = accept(connections->fd, (struct sockaddr*)&client_info, &client_info_len)) > 0) {
			connections = (struct pollfd*) realloc(connections, (++num_fds)*sizeof(struct pollfd));
			connections[num_fds-1].events = POLLIN;		 // There is data to read...
			connections[num_fds-1].fd = new_client_fd;	 // ...on the client socket
			printf("Client connected\n");
		}

		for (int i = 1; i < num_fds; i++) {
			if (connections[i].revents & POLLIN) {	// If there's data to read
				struct message_t* msg;
				if ((msg = network_receive(connections[i].fd)) == NULL) {
					close(connections[i].fd);
					printf("Client disconnected\n");
					memcpy(&connections[i], &connections[i+1], (--num_fds-i)*sizeof(struct pollfd));
					connections = (struct pollfd*) realloc(connections, num_fds*sizeof(struct pollfd));
					continue;
				}
				invoke(msg);
				network_send(connections[i].fd, msg);
			}
		}
	}
	return 0;
}

/* Esta função deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura message_t.
 */
struct message_t* network_receive(int client_socket) {
	char* buff = (char*)malloc(BUFFER_MAX_SIZE);
	int size = read_all(client_socket, &buff, BUFFER_MAX_SIZE);
	struct message_t* result = size > 0 ? message_t__unpack(NULL, size, (uint8_t*)buff) : NULL;
	free(buff);
	return result;
}

/* Esta função deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Libertar a memória ocupada por esta mensagem;
 * - Enviar a mensagem serializada, através do client_socket.
 */
int network_send(int client_socket, struct message_t* msg) {
	char* buffer = (char*)malloc(BUFFER_MAX_SIZE);
	int buffer_size = message_t__pack(msg, (uint8_t*)buffer);
	message_t__free_unpacked(msg, NULL);
	int num_bytes_written = write_all(client_socket, buffer, buffer_size);
	free(buffer);
	return num_bytes_written;
}

/* A função network_server_close() liberta os recursos alocados por
 * network_server_init().
 */
int network_server_close() {
	close(listening_socket);  // we might not need this one anymore, for further testing
	return 0;
}

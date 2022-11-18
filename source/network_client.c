// Trabalho realizado por:
// Pedro Trindade 56342
// Joao Santos 56380
// Marcus Gomes 56326

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "client_stub-private.h"
#include "message-private.h"
#include "network_client-private.h"
#include "network_client.h"
#include "sdmessage.pb-c.h"
#include "util.h"

/* Esta função deve:
 * - Obter o endereço do servidor (struct sockaddr_in) a base da
 *   informação guardada na estrutura rtree;
 * - Estabelecer a ligação com o servidor;
 * - Guardar toda a informação necessária (e.g., descritor do socket)
 *   na estrutura rtree;
 * - Retornar 0 (OK) ou -1 (erro).
 */
int network_connect(struct rtree_t* rtree) {
	// socket info
	struct sockaddr_in server_info = {0};
	server_info.sin_family = AF_INET;
	// Store this IP address in struct sockaddr_in
	inet_pton(AF_INET, rtree->address, &(server_info.sin_addr));
	server_info.sin_port = htons(atoi(rtree->port));
	socklen_t server_info_len = sizeof(server_info);

	// Ignore SIGPIPE signal so client doesn't crash if socket closes unexpectedly
	struct sigaction new_actn;
	new_actn.sa_handler = SIG_IGN;
	sigemptyset(&new_actn.sa_mask);
	new_actn.sa_flags = 0;
	sigaction(SIGPIPE, &new_actn, NULL);
	// socket
	int sfd = socket(AF_INET, SOCK_STREAM, 0);	// Using TCP/IP
	if (sfd < 0) {
		perror("socket");
		return -1;
	}
	rtree->socket_id = sfd;
	// connect
	int con = connect(sfd, (struct sockaddr*)&server_info, server_info_len);
	if (con == -1) {
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
struct message_t* network_send_receive(struct rtree_t* rtree, struct message_t* msg) {
	// Allocate memory for buffer
	char* buffer_to_send = (char*)malloc(message_t__get_packed_size(msg));

	// Generate buffer
	int buffer_size = message_t__pack(msg, (uint8_t*)buffer_to_send);

	// Write to network
	int bytes_written = write_all(rtree->socket_id, buffer_to_send, buffer_size);

	// Free buffer
	free(buffer_to_send);

	// Check for errors
	if (bytes_written < 0) {
		perror("Write failed!\n");
		return NULL;
	} else if (bytes_written == 0) {
		printf("Server closed.\n");
		return NULL;
	}
	// Read to buffer
	char* buffer_to_receive;
	int size = read_all2(rtree->socket_id, &buffer_to_receive);

	// Generate message
	struct message_t* response = message_t__unpack(NULL, size, (uint8_t*)buffer_to_receive);

	// Free buffer_to_receive
	free(buffer_to_receive);

	// Return response
	return response;
}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtree_t* rtree) {
	return close(rtree->socket_id);
}

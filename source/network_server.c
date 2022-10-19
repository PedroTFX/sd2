// #include "tree_skel.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <tree.h>

/* Função para preparar uma socket de receção de pedidos de ligação
 * num determinado porto.
 * Retornar descritor do socket (OK) ou -1 (erro).
 */
int network_server_init(short port){
    //socket info struct
    struct sockaddr_in server_info = {0};
    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(port);
    socklen_t server_info_len = sizeof(server_info);
    
    //socket
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd < 0){
        perror("socket");
        return -1;
    }
    printf("socket created\n");

    //bind to addr
    if(bind(sfd, (struct sockaddr*)&server_info, server_info_len) < 0){
        perror("bind");
        return -1;
    }
    printf("socket binded\n");

    // listen for connections
    if(listen(sfd, 0) < 0){   //right declaration
        perror("listen");
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
int network_main_loop(int listening_socket){

    //accept
    struct sockaddr client_info = {0};
    socklen_t client_info_len = sizeof(client_info); 

    int cfd = accept(listening_socket, &client_info, &client_info_len);
    if(cfd < 0){
        perror("accept");
        return -1;
    }
    printf("accept\n");

    close(listening_socket); //we dont need this one anymore
    

    return cfd;
}

/* Esta função deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura message_t.
 */
struct message_t *network_receive(int client_socket){ 

    //decript msm
}

/* Esta função deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Libertar a memória ocupada por esta mensagem;
 * - Enviar a mensagem serializada, através do client_socket.
 */
int network_send(int client_socket, struct message_t *msg){
    char *hello = "Hello world\n";

    ssize_t sent = send(client_socket,(void *) hello, strlen(hello) - 1, 0);
    close(client_socket);
}

/* A função network_server_close() liberta os recursos alocados por
 * network_server_init().
 */
int network_server_close(){
    //no fucking idea how to close server
}

int main(int argc, char const *argv[])
{
    int sfd = network_server_init(1337); //existem ports que n se pod escolher
    int cfd = network_main_loop(sfd);
    network_send(cfd, NULL);

    return 0;
}

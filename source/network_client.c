#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <client_stub-private.h>

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
    server_info.sin_addr.s_addr = htonl(0x7f000001);
    server_info.sin_port = htons(1337);

    socklen_t server_info_len = sizeof(server_info);

    //socket
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd < 0){
        perror("socket");
        return -1;
    }

    //connect
    if(connect(sfd, (struct sockaddr*)&server_info, server_info_len)){
        perror("connect");
        return -1;
    }

    //client stuff
    char buffer[1024];
    ssize_t recvd = recv(sfd, buffer, 1024 - 1, 0);
    printf("%s", buffer);

    //close
    close(sfd);
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

}

/* A função network_close() fecha a ligação estabelecida por
 * network_connect().
 */
int network_close(struct rtree_t * rtree){

}

int main(int argc, char const *argv[])
{
    // struct rtree* server_tree = {0};
    // server_tree->port = 1337;
    // server_tree->root = tree_create();

    network_connect(NULL);
    return 0;
}

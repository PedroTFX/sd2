#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"
#include "entry.h"
#include "network_client.h"
#include "client_stub.h"
#include "client_stub-private.h"

int main(int argc, char const *argv[]) {
	if (argc < 2) {
		printf("Usage: ./tree_client <server>:<port>\n");
	}

	// connect to server and port
	char* adrressport[255];
	printf(argv[1]);
	printf("\n");

	sprintf(adrressport, argv[1], "%s:%s");
	struct rtree_t* rtree = rtree_connect(adrressport);

	//printf("lista de comandos disponiveis:\n");
	printf("pedido:\n");
	char *input[999];
	fgets(input, 999, stdin);
	char *token = strtok(input, " ");
	if (strcmp(token, "quit")== 0) {
        rtree_disconnect(r_tree);
    } else if(strcmp(token, "size")== 0) {
		rtree_size(r_tree);
	} else if (strcmp(token, "height")== 0) {
		rtree_height(r_tree);
	} else if (strcmp(token, "getkeys")==0) {
		rtree_get_keys(r_tree);
	} else if (strcmp(token, "getvalues")==0) {
		rtree_get_values(r_tree);
	} else if (strcmp(token, "del")==0) {
		token = strtok(NULL, " ");
		rtree_del(rtree, token);
	} else if (strcmp(token, "get")==0) {
		token = strtok(NULL, " ");
		rtree_get(rtree,token);
	} else if(strcmp(token,"put")==0){
		char* key = strtok(NULL, " ");
		token = strtok(NULL, " ");
		rtree_put(rtree, key, token);
	} else{
		printf("xau\n")
	}

	while (token != NULL) {
		printf("%s\n", token);
		token = strtok(NULL, " ");
	}

	printf("%s", input);
	// struct entry_t* entry;
	// rtree_put(r_tree, entry);

	// create request for action

	// close connection
	// rtree_disconnect(r_tree);
	return 0;
}

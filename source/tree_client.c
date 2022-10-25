#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"
#include "entry.h"
#include "network_client.h"
#include "client_stub.h"
#include "client_stub-private.h"
#include "tree_client-private.h"

#define PUT "put"
#define GET "get"
#define QUIT "quit"
int main(int argc, char const* argv[]) {
	if (argc < 2) {
		printf("Usage: ./tree_client <server>:<port>\n");
		return -1;
	}

	struct rtree_t* r_tree = rtree_connect(argv[1]);

	if (r_tree == NULL) {
		perror("could not connect client\n");
		return -1;
	}

	char option[1024];
	do {
		showMenu();
		readOption(option, 1024);

		if (commandIsPut(option)) {
			executePut(r_tree, option);
		} else if (commandIsGet(option)) {
			executeGet(r_tree, option);
		}/*  else if (strcmp(command, "del") == 0) {
			command = strtok(NULL, " ");
			rtree_del(r_tree, command);
		} else if (strcmp(command, "size") == 0) {
			rtree_size(r_tree);
		} else if (strcmp(command, "height") == 0) {
			rtree_height(r_tree);
		} else if (strcmp(command, "getkeys") == 0)
			rtree_get_keys(r_tree);
		} else if (strcmp(command, "getvalues") == 0) {
			rtree_get_values(r_tree);
		} */
	} while (strncmp(option, QUIT, strlen(QUIT)) != 0);

	rtree_disconnect(r_tree);
	printf("Client exiting. Bye.\n");

	return 0;
}

void showMenu() {
	printf("\n");
	printf("put <key> <value>\n");
	printf("get <key>\n");
	printf("del <key>\n");
	printf("size\n");
	printf("height\n");
	printf("getkeys\n");
	printf("getvalues\n");
	printf("quit\n");
	printf("Option: ");
}

void readOption(char* input, int size) {
	fgets(input, size, stdin); // char* input <- stdin até um máximo de size bytes
	input[strlen(input)-1] = '\0';
}

int commandIsPut(char* option) {
	return strncmp(option, PUT, strlen(PUT)) == 0;
}

int commandIsGet(char* option){
	return strncmp(option, GET, strlen(GET)) == 0;
}

void executePut(struct rtree_t* r_tree, char* option) {
	strtok(option, " ");
	char* key = strtok(NULL, " ");
	char* value = strtok(NULL, " ");
	struct data_t* data = data_create2(strlen(value), value);
	struct entry_t* entry = entry_create(key, data);
	if (rtree_put(r_tree, entry) == -1) {
		printf("\nput failed\n");
		return;
	}
	printf("\nput successful\n");
}

void executeGet(struct rtree_t* r_tree, char* option) {
	strtok(option, " ");
	char* key = strtok(NULL, " ");
	struct data_t* value = rtree_get(r_tree, key);
	if (value == NULL) {
		printf("\nget failed\n");
		return;
	}
	char* buffer = (char*) calloc(sizeof(char), value->datasize + 1);
	strncpy(buffer, value->data, value->datasize); //ja coloca "\0" automaticamente
	printf("\nNumber of bytes: %d\n", value->datasize);
	printf("Bytes: %s\n", buffer);
	free(buffer);
}

// tentar imprimir o pedido do cliente

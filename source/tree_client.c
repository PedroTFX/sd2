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
#define DEL "del"
#define QUIT "quit"
#define SIZE "size"
#define HEIGHT "height"
#define GET_KEYS "getkeys"
#define GET_VALUES "getvalues"

int main(int argc, char const* argv[]) {
	if (argc < 2) {
		printf("Usage: ./tree_client <server>:<port>\n");
		return -1;
	}

	struct rtree_t* rtree = rtree_connect(argv[1]);

	if (rtree == NULL) {
		perror("could not connect client\n");
		return -1;
	}

	char option[1024];
	do {
		showMenu();
		readOption(option, 1024);

		if (commandIsPut(option)) {
			executePut(rtree, option);
		} else if (commandIsGetKeys(option)) {
			executeGetKeys(rtree);
		} else if (commandIsGetValues(option)) {
			executeGetValues(rtree);
		} else if (commandIsGet(option)) {
			executeGet(rtree, option);
		}  else if (commandIsDel(option)) {
			executeDel(rtree, option);
		}/* else if (strcmp(command, "size") == 0) {
			rtree_size(rtree);
		} else if (strcmp(command, "height") == 0) {
			rtree_height(rtree);
		}
		*/
	} while (strncmp(option, QUIT, strlen(QUIT)) != 0);

	rtree_disconnect(rtree);
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

int commandIsDel(char* option){
	return strncmp(option, DEL, strlen(DEL)) == 0;
}

int commandIsGetKeys(char* option) {
	return strncmp(option, GET_KEYS, strlen(GET_KEYS)) == 0;
}

int commandIsGetValues(char* option) {
	return strncmp(option, GET_VALUES, strlen(GET_VALUES)) == 0;
}

void executePut(struct rtree_t* rtree, char* option) {
	strtok(option, " ");
	char* key = strtok(NULL, " ");
	char* value = strtok(NULL, " ");
	struct data_t* data = data_create2(strlen(value), value);
	struct entry_t* entry = entry_create(key, data);
	if (rtree_put(rtree, entry) == -1) {
		printf("\nput failed\n");
		return;
	}
	printf("\nput successful\n");
}

void executeGet(struct rtree_t* rtree, char* option) {
	strtok(option, " ");
	char* key = strtok(NULL, " ");
	struct data_t* value = rtree_get(rtree, key);
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

void executeDel(struct rtree_t* rtree, char* option) {
	strtok(option, " ");
	char* key = strtok(NULL, " ");
	int result = rtree_del(rtree, key);
	if (result == -1) {
		printf("\nDel failed\n");
		return;
	}
	printf("\nDel successful\n");
}

void executeGetKeys(struct rtree_t* rtree) {
	char** keys = rtree_get_keys(rtree);
	if(keys == NULL) {
		printf("There was an error executing get_keys() on the server.\n");
		return;
	}

	if(keys[0] == NULL) {
		printf("\nThere are no keys.\n");
		return;
	}

	printf("\nKeys: \n");
	int i = 0;
	while (keys[i] != NULL) {
		printf("%s\n", keys[i++]);
	}
}

void executeGetValues(struct rtree_t* rtree) {
	char** keys = rtree_get_values(rtree);
	if(keys == NULL) {
		printf("There was an error executing get_values() on the server.\n");
		return;
	}

	if(keys[0] == NULL) {
		printf("\nThere are no values.\n");
		return;
	}

	printf("\nKeys: \n");
	int i = 0;
	while (keys[i] != NULL) {
		printf("%s\n", keys[i++]);
	}
}

// tentar imprimir o pedido do cliente

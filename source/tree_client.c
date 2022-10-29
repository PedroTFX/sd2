#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client_stub-private.h"
#include "client_stub.h"
#include "data.h"
#include "entry.h"
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
		} else if (commandIsDel(option)) {
			executeDel(rtree, option);
		} else if (commandIsSize(option)) {
			executeSize(rtree);
		} else if (commandIsHeight(option)) {
			executeHeight(rtree);
		}
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
	fgets(input, size, stdin);	// char* input <- stdin até um máximo de size bytes
	input[strlen(input) - 1] = '\0';
}

int commandIsPut(char* option) {
	return strncmp(option, PUT, strlen(PUT)) == 0;
}

int commandIsGet(char* option) {
	return strncmp(option, GET, strlen(GET)) == 0;
}

int commandIsDel(char* option) {
	return strncmp(option, DEL, strlen(DEL)) == 0;
}

int commandIsSize(char* option) {
	return strncmp(option, SIZE, strlen(SIZE)) == 0;
}

int commandIsHeight(char* option) {
	return strncmp(option, HEIGHT, strlen(HEIGHT)) == 0;
}

int commandIsGetKeys(char* option) {
	return strncmp(option, GET_KEYS, strlen(GET_KEYS)) == 0;
}

int commandIsGetValues(char* option) {
	return strncmp(option, GET_VALUES, strlen(GET_VALUES)) == 0;
}

void executePut(struct rtree_t* rtree, char* option) {
	strtok(option, " ");
	char* key = strdup(strtok(NULL, " "));
	char* value = strdup(strtok(NULL, " "));
	struct entry_t* entry = entry_create(key, data_create2(strlen(value), value));
	if (rtree_put(rtree, entry) == -1) {
		printf("\nput failed\n");
		return;
	}
	entry_destroy(entry);
	printf("\n#######put successful#######\n");
}

void executeGet(struct rtree_t* rtree, char* option) {
	strtok(option, " ");
	char* key = strdup(strtok(NULL, " "));
	struct data_t* value = rtree_get(rtree, key);
	free(key);
	if (value == NULL) {
		printf("\nget failed\n");
		return;
	}
	char* buffer = (char*)calloc(sizeof(char), value->datasize + 1);
	strncpy(buffer, value->data, value->datasize);	// ja coloca "\0" automaticamente
	printf("\n###Number of bytes: %d\n", value->datasize);
	printf("###Bytes: %s\n", buffer);
	data_destroy(value);
	free(buffer);
}

void executeDel(struct rtree_t* rtree, char* option) {
	strtok(option, " ");
	char* key = strdup(strtok(NULL, " "));
	int result = rtree_del(rtree, key);
	if (result == -1) {
		printf("\nDel failed\n");
		return;
	}
	free(key);
	printf("\n #######Del successful####### \n");
}

void executeSize(struct rtree_t* rtree) {
	int result = rtree_size(rtree);
	if (result == -1) {
		printf("\nSize failed\n");
		return;
	}
	printf("\n #######Size successful####### \n");
	printf("Size: %d\n", result);
}

void executeHeight(struct rtree_t* rtree) {
	int result = rtree_height(rtree);
	if (result == -1) {
		printf("\nHeight failed\n");
		return;
	}
	printf("\n #######Height successful####### \n");
	printf("Height: %d\n", result);
}

void executeGetKeys(struct rtree_t* rtree) {
	char** keys = rtree_get_keys(rtree);
	if (keys == NULL) {
		printf("There was an error executing get_keys() on the server.\n");
		return;
	}

	if (keys[0] == NULL) {
		printf("\nThere are no keys.\n");
		return;
	}

	printf("\nKeys: \n");
	int i = 0;
	while (keys[i] != NULL) {
		printf("%s\n", keys[i]);
		free(keys[i++]);
	}
	free(keys);
}

void executeGetValues(struct rtree_t* rtree) {
	struct data_t** values = (struct data_t**)rtree_get_values(rtree);
	if (values == NULL) {
		printf("There was an error executing get_values() on the server.\n");
		return;
	}

	if (values[0] == NULL) {
		printf("\nThere are no values.\n");
		return;
	}

	printf("\nValues: \n");
	int i = 0;
	char* str;
	while (values[i] != NULL) {
		str = strdup((char*)values[i]->data);
		printf("%s\n", str);
		free(values[i]->data);
		free(values[i]);
		i++;
	}
	free(str);
	free(values);
}

// tentar imprimir o pedido do cliente

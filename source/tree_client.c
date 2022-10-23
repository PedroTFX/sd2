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
		readOption(option);

		if (commandIsPut(option)) {
			executePut(r_tree, option);
		} /* else if (strcmp(command, "get") == 0) {
			command = strtok(NULL, " ");
			rtree_get(r_tree, command);
		} else if (strcmp(command, "del") == 0) {
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

void readOption(char* input) {
	fgets(input, 999, stdin);
}

int commandIsPut(char* option) {
	return strncmp(option, PUT, strlen(PUT)) == 0;
}

void executePut(struct rtree_t* r_tree, char* option) {
	strtok(option, " ");
	char* key = strtok(NULL, " ");
	char* value = strtok(NULL, " ");
	struct data_t* data = data_create2(strlen(value), value);
	struct entry_t* entry = entry_create(key, data);
	if (rtree_put(r_tree, entry) == -1) {
		printf("put failed\n");
		return;
	}
	printf("put successful\n");
}

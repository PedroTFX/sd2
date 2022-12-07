// Trabalho realizado por:
// Pedro Trindade 56342
// Joao Santos 56380
// Marcus Gomes 56326

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client_stub-private.h"
#include "client_stub.h"
#include "client_zookeeper-private.h"
#include "data.h"
#include "entry.h"
#include "tree_client-private.h"
#include "bubble_sort-private.h"

#define PUT "put "
#define GET "get "
#define DEL "del "
#define QUIT "quit"
#define SIZE "size"
#define HEIGHT "height"
#define GET_KEYS "getkeys"
#define GET_VALUES "getvalues"
#define VERIFY "verify "
#define RANDOM "random"

zhandle_t* zh;
struct rtree_t* head = NULL;
struct rtree_t* tail = NULL;

int main(int argc, char const* argv[]) {
	// Ignore SIGPIPE signal so server doesn't crash if socket closes unexpectedly
	struct sigaction new_actn;
	new_actn.sa_handler = SIG_IGN;
	sigemptyset(&new_actn.sa_mask);
	new_actn.sa_flags = 0;
	sigaction(SIGPIPE, &new_actn, NULL);

	if (argc < 2) {
		printf("Usage: ./tree_client <zookeeper server>:<zookeeper port>\n");
		return -1;
	}

	// Connect to ZooKeeper
	char* root_path = "/chain";
	zh = zk_connect(argv[1], root_path);
	if (zh == NULL) {
		fprintf(stderr, "Error connecting to ZooKeeper!\n");
		exit(EXIT_FAILURE);
	}

	// Get children list
	struct watcher_ctx watcher_ctx;
	watcher_ctx.callback = select_head_and_tail_servers;
	zk_get_children(zh, &watcher_ctx);

	char option[1024];
	do {
		showMenu();
		readOption(option, 1024);
		executeCommand(option);
	} while (strncmp(option, QUIT, strlen(QUIT)) != 0);
	zk_disconnect(zh);	// Disconnect from ZooKeeper
	printf("Client exiting. Bye.\n");
	return 0;
}

// Process children list
void select_head_and_tail_servers(zoo_string* children_list, char* root_path, zhandle_t* zh) {

	printf("Callback function was called on the client!\n");
	printf("antes de ordenar\n");
	for (int i = 0; i < children_list->count; i++) {
		printf("%s\n", children_list->data[i]);
	}
	bubble_sort(children_list->data, children_list->count);
	printf("depois de ordenar\n");
	for (int i = 0; i < children_list->count; i++) {
		printf("%s\n", children_list->data[i]);
	}
	if (children_list->count > 0) {
		// Get next node's IP and port
		int watch = 0;
		int node_metadata_length = ZDATALEN;
		char* node_metadata = malloc(node_metadata_length * sizeof(char));
		struct Stat* stat = NULL;
		char node_path[120] = "";
		strcat(node_path, root_path);
		strcat(node_path, "/");
		strcat(node_path, children_list->data[0]);
		if (zoo_get(zh, node_path, watch, node_metadata, &node_metadata_length, stat) != ZOK) {
			fprintf(stderr, "Error getting new node's metadata at %s!\n", root_path);
		}

		head = rtree_connect(node_metadata);

		node_path[0] = '\0';
		strcat(node_path, root_path);
		strcat(node_path, "/");
		strcat(node_path, children_list->data[children_list->count - 1]);
		if (zoo_get(zh, node_path, watch, node_metadata, &node_metadata_length, stat) != ZOK) {
			fprintf(stderr, "Error getting new node's metadata at %s!\n", root_path);
		}

		tail = rtree_connect(node_metadata);
	}
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
	printf("verify <operation number>\n");
	printf("quit\n");
	printf("Option: ");
}

void readOption(char* input, int size) {
	fgets(input, size, stdin);	// char* input <- stdin até um máximo de size bytes
	input[strlen(input) - 1] = '\0';
}

void executeCommand(char* option) {
	if (commandIsPut(option)) {
		executePut(option);
	} else if (commandIsGetKeys(option)) {
		executeGetKeys();
	} else if (commandIsGetValues(option)) {
		executeGetValues();
	} else if (commandIsGet(option)) {
		executeGet(option);
	} else if (commandIsDel(option)) {
		executeDel(option);
	} else if (commandIsSize(option)) {
		executeSize();
	} else if (commandIsHeight(option)) {
		executeHeight();
	} else if (commandIsVerify(option)) {
		executeVerify(option);
	} else {
		printf("\nInvalid operation\n");
	}
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

int commandIsVerify(char* option) {
	return strncmp(option, VERIFY, strlen(VERIFY)) == 0;
}

int commandIsRandom(char* option) {
	return strncmp(option, RANDOM, strlen(RANDOM)) == 0;
}

void executePut(char* option) {
	strtok(option, " ");
	char* key = strdup(strtok(NULL, " "));
	char* value = strdup(strtok(NULL, " "));
	struct entry_t* entry = entry_create(key, data_create2(strlen(value), value));
	int op_num = rtree_put(head, entry);
	entry_destroy(entry);
	if (op_num == -1) {
		printf("\nPut failed.\n");
		return;
	}
	printf("\n#######Put operation queued with number %d#######\n", op_num);
}

void executeGet(char* option) {
	strtok(option, " ");
	char* key = strdup(strtok(NULL, " "));
	struct data_t* value = rtree_get(tail, key);
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

void executeDel(char* option) {
	strtok(option, " ");
	char* key = strdup(strtok(NULL, " "));
	int op_num = rtree_del(head, key);
	free(key);
	if (op_num == -1) {
		printf("\nDel failed\n");
		return;
	}
	printf("\n#######Del operation queued with number %d#######\n", op_num);
}

void executeSize() {
	int result = rtree_size(tail);
	if (result == -1) {
		printf("\nSize failed\n");
		return;
	}
	printf("\n #######Size successful####### \n");
	printf("Size: %d\n", result);
}

void executeHeight() {
	int result = rtree_height(tail);
	if (result == -1) {
		printf("\nHeight failed\n");
		return;
	}
	printf("\n #######Height successful####### \n");
	printf("Height: %d\n", result);
}

void executeGetKeys() {
	char** keys = rtree_get_keys(tail);
	if (keys == NULL) {
		printf("\nGetkeys failed\n");
		return;
	}

	if (keys[0] == NULL) {
		printf("\nThere are no keys.\n");
		free(keys);
		return;
	}

	printf("\nKeys: \n");
	int i = 0;
	while (keys[i] != NULL) {
		printf("%s\n", keys[i]);
		free(keys[i++]);
	}
	free(keys[i]);
	free(keys);
}

void executeGetValues() {
	struct data_t** values = (struct data_t**)rtree_get_values(tail);
	if (values == NULL) {
		printf("\nGetvalues failed\n");
		return;
	}

	if (values[0] == NULL) {
		printf("\nThere are no values.\n");
		free(values);
		return;
	}

	printf("\nValues: \n");
	int i = 0;
	while (values[i] != NULL) {
		for (int j = 0; j < values[i]->datasize; j++) {
			printf("%c", ((char*)values[i]->data)[j]);
		}
		printf("\n");
		data_destroy(values[i]);
		i++;
	}
	free(values);
}

void executeVerify(char* option) {
	strtok(option, " ");
	int op_n = atoi(strtok(NULL, ""));
	int verified = rtree_verify(tail, op_n);
	if (verified == -1) {
		printf("\nVerify failed\n");
		return;
	}
	if (verified) {
		printf("The operation %d was completed!\n", op_n);
	} else {
		printf("The operation %d was not completed yet!\n", op_n);
	}
}

void sig_pipe_handler(int signal) {
	printf("Client exiting after server crash. Bye.\n");
	zk_disconnect(zh);
}

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
#define VERIFY "verify"
#define RANDOM "random"

struct rtree_t* rtree;

int main(int argc, char const* argv[]) {
	// Ignore SIGPIPE signal so server doesn't crash if socket closes unexpectedly
	struct sigaction new_actn;
	new_actn.sa_handler = SIG_IGN;
	sigemptyset(&new_actn.sa_mask);
	new_actn.sa_flags = 0;
	sigaction(SIGPIPE, &new_actn, NULL);

	if (argc < 2) {
		printf("Usage: ./tree_client <server>:<port>\n");
		return -1;
	}

	rtree = rtree_connect(argv[1]);
	if (rtree == NULL) {
		perror("could not connect client\n");
		return -1;
	}

	char option[1024];
	do {
		showMenu();
		readOption(option, 1024);
		executeCommand(rtree, option);
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
	printf("verify <operation number>\n");
	printf("quit\n");
	printf("Option: ");
}

void readOption(char* input, int size) {
	fgets(input, size, stdin);	// char* input <- stdin até um máximo de size bytes
	input[strlen(input) - 1] = '\0';
}

void executeCommand(struct rtree_t* rtree, char* option) {
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
	} else if (commandIsVerify(option)) {
		executeVerify(rtree, option);
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

void executePut(struct rtree_t* rtree, char* option) {
	strtok(option, " ");
	char* key = strdup(strtok(NULL, " "));
	char* value = strdup(strtok(NULL, " "));
	struct entry_t* entry = entry_create(key, data_create2(strlen(value), value));
	int op_num = rtree_put(rtree, entry);
	entry_destroy(entry);
	if (op_num == -1) {
		printf("\nPut failed.\n");
		return;
	}
	printf("\n#######Put operation queued with number %d#######\n", op_num);
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
	int op_num = rtree_del(rtree, key);
	free(key);
	if (op_num == -1) {
		printf("\nDel failed\n");
		return;
	}
	printf("\n#######Del operation queued with number %d#######\n", op_num);
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

void executeGetValues(struct rtree_t* rtree) {
	struct data_t** values = (struct data_t**)rtree_get_values(rtree);
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

void executeVerify(struct rtree_t* rtree, char* option) {
	strtok(option, " ");
	int op_n = atoi(strtok(NULL, ""));
	int verified = rtree_verify(rtree, op_n);
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

void sig_pipe_handler(int signal){
	printf("Client exiting after server crash. Bye.\n");
	rtree_disconnect(rtree);
}

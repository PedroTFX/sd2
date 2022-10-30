#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "tree_server-private.h"
#include "network_server.h"

int main(int argc, char const *argv[]) {
	if (argc != 2) {
		printf("Usage: tree-server <port>\n");
		return -1;
	}
	short port = (short)atoi(argv[1]);
	if (port == 0) {
		printf("Usage: tree-server <port>\n");
		printf("Port should be a number.\n");
		return -1;
	}

	// Initialize server
	int listening_socket;
	if ((listening_socket = network_server_init(port)) == -1) {
		printf("Could not listen on port %u\n", port);
		return -1;
	}

	// Initialize tree
	if(tree_skel_init()) {
		printf("Could not initialize tree.\n");
		if(network_server_close() != 0) {
			printf("Error in network_server_close()\n");
		}else{
			printf("\nServer Closed Sucessfuly\n");
		}
		return -1;
	}

	// Listen to interrupt signal
	signal(SIGINT, tree_server_close);

	// Listen to client requests
	if(network_main_loop(listening_socket) == -1) {
		printf("Error in network_main_loop()\n");
	}

	return 0;
}

void tree_server_close(int signum) {
	// Close server
	if(network_server_close() != 0) {
		printf("Error in network_server_close()\n");
	}else{
		printf("\nServer Closed Sucessfuly\n");
	}
	// Destroy tree
	tree_skel_destroy();
}

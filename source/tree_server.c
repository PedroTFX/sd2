#include <stdlib.h>
#include <stdio.h>

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

	int listening_socket = network_server_init(port);
	if (listening_socket == -1) {
		printf("Could not listen on port %u\n", port);
		return -1;
	}

	tree_skel_init();
	if(network_main_loop(listening_socket) != 0) {
		printf("Error in network_main_loop()\n");
	}
	if(network_server_close() != 0) {
		printf("Error in network_server_close()\n");
	}
	tree_skel_destroy();

	return 0;
}


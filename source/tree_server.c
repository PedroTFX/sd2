#include <stdlib.h>
#include <stdio.h>
#include "tree_skel.h"
#include "network_server.h"





/* int main(int argc, char const *argv[])
{
    //open server on port

    //wait for read

    //do action

    //closing??

    return 0;
} */

int main(int argc, char const *argv[]) {
	if (argc != 2) {
		printf("Usage: tree-server <port>\n");
		return;
	}
	short port = (short)atoi(argv[1]);
	if (port == 0) {
		printf("Usage: tree-server <port>\n");
		printf("Port should be a number.\n");
		return;
	}

	int listening_socket = network_server_init(port);
	if (listening_socket == -1) {
		print("Could not listen on port %u\n");
		return;
	}

	if(network_main_loop(listening_socket) != 0) {
		printf("Error in network_main_loop()\n");
	}

	if(network_server_close() != 0) {
		printf("Error in network_server_close()\n");
	}
	return 0;
}


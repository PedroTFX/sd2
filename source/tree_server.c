// Trabalho realizado por:
// Pedro Trindade 56342
// Joao Santos 56380
// Marcus Gomes 56326
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "client_zookeeper-private.h"
#include "network_server.h"
#include "tree_server-private.h"
#include "tree_skel-private.h"

int main(int argc, const char* argv[]) {
	if (argc != 3) {
		printf("Usage: tree-server <port> <zookeeper address>:<zookeeper port>\n");
		return -1;
	}

	// Initialize tree
	if (tree_skel_init(1)) {
		printf("Could not initialize tree.\n");
		if (network_server_close() != 0) {
			printf("Error in network_server_close()\n");
		}
		return -1;
	}

	// Connect to ZooKeeper to forward client requests
	zook_address_port = argv[2];
	zh = zk_connect(zook_address_port);
	if (zh == NULL) {
		fprintf(stderr, "Error connecting to ZooKeeper!\n");
		exit(EXIT_FAILURE);
	}

	// Create root node if it doesn't exist
	zk_create_root_node_if_doesnt_exist(zh);

	// Register server in ZooKeeper
	const char* server_port = argv[1];
	zk_register_server(zh, server_port);

	// Get children list
	struct watcher_ctx watcher_ctx;
	watcher_ctx.callback = select_next_server;
	zk_get_children(zh, &watcher_ctx);

	// Listen to client request
	short port = (short)atoi(server_port);
	if (port == 0) {
		printf("Usage: tree-server <server port> <zookeeper address>:<zookeeper port>\n");
		printf("Port should be a number.\n");
		return -1;
	}
	int listening_socket;
	if ((listening_socket = network_server_init(port)) == -1) {
		printf("Could not listen on port %u\n", port);
		return -1;
	}

	// Listen to interrupt signal
	signal(SIGINT, tree_server_close);

	// Receive client requests, process them, and send replies
	if (network_main_loop(listening_socket) == -1) {
		printf("Error in network_main_loop()\n");
	}

	return 0;
}

void tree_server_close(int signum) {
	printf("Closing\n");

	// Disconnect from ZooKeeper
	zk_disconnect(zh);

	// Close server
	if (network_server_close() != 0) {
		printf("Error in network_server_close()\n");
	}

	printf("Network server close\n");

	// Destroy tree
	tree_skel_destroy();
}

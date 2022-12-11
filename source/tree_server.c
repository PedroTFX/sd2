// Trabalho realizado por:
// Pedro Trindade 56342
// Joao Santos 56380
// Marcus Gomes 56326
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client_stub.h"
#include "client_stub-private.h"
#include "client_zookeeper-private.h"
#include "network_server.h"
#include "tree_server-private.h"
#include "tree_skel-private.h"
#include "bubble_sort-private.h"

zhandle_t* zh;

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

	// Listen to client request
	const char* server_port = argv[1];
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

	// Connect to ZooKeeper to forward client requests
	char* root_node = "/chain";
	zh = zk_connect(argv[2], root_node);
	if (zh == NULL) {
		fprintf(stderr, "Error connecting to ZooKeeper!\n");
		exit(EXIT_FAILURE);
	}

	// Create root node if it doesn't exist
	zk_create_root_node_if_doesnt_exist(zh);

	// Register server in ZooKeeper
	zk_register_server(zh, server_port);

	// Get children list
	struct watcher_ctx watcher_ctx;
	watcher_ctx.callback = select_next_server;
	zk_get_children(zh, &watcher_ctx);

	// Listen to interrupt signal
	signal(SIGINT, tree_server_close);

	// Receive client requests, process them, and send replies
	if (network_main_loop(listening_socket) == -1) {
		printf("Error in network_main_loop()\n");
	}

	return 0;
}

void select_next_server(zoo_string* children_list, char* root_path, zhandle_t* zh) {
	// Process children list
	printf("Callback function was called on the server!\n");
	printf("antes de ordenar\n");
	for (int i = 0; i < children_list->count; i++) {
		printf("%s\n", children_list->data[i]);
	}
	bubble_sort(children_list->data, children_list->count);
	printf("depois de ordenar\n");
	for (int i = 0; i < children_list->count; i++) {
		printf("%s\n", children_list->data[i]);
	}
	int i;
	for (i = 0; i < children_list->count; i++) {
		if(i == 0 && strcmp(children_list->data[i], zk_node_id) == 0) {
			printf("I'm the head!\n");
		}

		// If we find next node that is not the previous one
		if (strcmp(children_list->data[i], zk_node_id) > 0) {
			// Get next node's IP and port
			int watch = 0;
			int node_metadata_length = ZDATALEN;
			char* node_metadata = malloc(node_metadata_length * sizeof(char));
			struct Stat* stat = NULL;
			char node_path[120] = "";
			strcat(node_path, root_path);
			strcat(node_path, "/");
			strcat(node_path, children_list->data[i]);
			if (zoo_get(zh, node_path, watch, node_metadata, &node_metadata_length, stat) != ZOK) {
				fprintf(stderr, "Error getting new node's metadata at %s!\n", root_path);
			}

			// Connect to the next server
			struct rtree_t* temp_next_server = rtree_connect(node_metadata);
			if (temp_next_server == NULL) {
				fprintf(stderr, "Error connecting to the next server %s:%s!\n", temp_next_server->address, temp_next_server->port);
			} else {
				// If we're already connected
				if(next_server != NULL) {
					rtree_disconnect(next_server);
				}

				// Switch to the new connection
				next_server = temp_next_server;
				fprintf(stdout, "Connected to the next server %s:%s!\n", next_server->address, next_server->port);
			}

			// We're done, leave cycle
			free(node_metadata);
			break;
		}
	}

	// If we didn't find a node higher than ours, then there's no next node
	if (i == children_list->count) {
		// If we're already connected
		if(next_server != NULL) {
			rtree_disconnect(next_server);
			next_server = NULL;
		}
		printf("I'm the tail!\n");
	}
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

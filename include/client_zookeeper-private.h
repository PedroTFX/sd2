#ifndef _CLIENT_ZOOKEEPER_PRIVATE_H
#define _CLIENT_ZOOKEEPER_PRIVATE_H

#include <zookeeper.h>

/* ZooKeeper Znode Data Length (1MB, the max supported) */
#define ZDATALEN 1024 * 1024

typedef struct String_vector zoo_string;

typedef void (*call_back_fn)(zoo_string* children_list, char* root_path, zhandle_t* zh);

struct watcher_ctx {
	call_back_fn callback;
};

char zk_node_id[ZDATALEN];

zhandle_t* zk_connect(const char* address_port, char* rp);

void zk_create_root_node_if_doesnt_exist(zhandle_t* zh);

void zk_register_server(zhandle_t* zh, const char* server_port);

void zk_print_nodes(zoo_string* children_list);


void zk_get_children(zhandle_t* zh, void* watcher_ctx);

void zk_child_watcher(zhandle_t* zh, int type, int state, const char* zpath, void* watcher_ctx);

void get_ip_address(char* ip_address);

void zk_disconnect(zhandle_t* zh);

#endif

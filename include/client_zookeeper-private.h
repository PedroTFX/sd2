#ifndef _CLIENT_ZOOKEEPER_PRIVATE_H
#define _CLIENT_ZOOKEEPER_PRIVATE_H

#include <zookeeper.h>

typedef struct String_vector zoo_string;
/* zoo_string* children_list, char* root_path */
typedef void (*call_back_fn)(zoo_string* children_list, char* root_path, zhandle_t* zh);

struct watcher_ctx {
	call_back_fn callback;
};

char* zk_node_id;

/* struct head_tail_servers {
	struct rtree_t* head;
	struct rtree_t* tail;
};

struct head_tail_servers* servers; */

zhandle_t* zk_connect(const char* address_port);

void zk_create_root_node_if_doesnt_exist(zhandle_t* zh);

void zk_register_server(zhandle_t* zh, const char* server_port);

void zk_print_nodes(zoo_string* children_list);

char* zk_get_full_path(char* root, char* child);

void zk_get_children(zhandle_t* zh, void* watcher_ctx);

void zk_child_watcher(zhandle_t* zh, int type, int state, const char* zpath, void* watcher_ctx);

void zk_disconnect(zhandle_t* zh);

#endif

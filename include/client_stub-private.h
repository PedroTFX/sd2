#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "client_zookeeper-private.h"

extern const char* zook_address_port;
extern const char* server_port;

struct rtree_t {
	char* address;
	char* port;
	int socket_id;
};

void select_head_and_tail_servers(zoo_string* children_list, char* root_path, zhandle_t* zh);

#endif

//client_stub-private
#ifndef _CLIENT_STUB_PRIVATEH
#define _CLIENT_STUB_PRIVATE_H

#include "data.h"
#include "entry.h"
#include "tree.h"
#include "client_stub.h"

struct rtree_t {
	uint16_t port;
	uint32_t address;
    int socket_id;
    struct tree_t* root;
};

#endif

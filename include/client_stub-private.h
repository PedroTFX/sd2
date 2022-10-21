//client_stub-private
#ifndef _CLIENT_STUB_PRIVATEH
#define _CLIENT_STUB_PRIVATE_H

#include "data.h"
#include "entry.h"
#include "tree.h"
#include "client_stub.h"

struct rtree_t{
    int port;
    struct tree_t* root;
};

#endif

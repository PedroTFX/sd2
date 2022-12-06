#ifndef _TREE_SERVER_PRIVATE_H
#define _TREE_SERVER_PRIVATE_H

#include "client_stub-private.h"

extern struct rtree_t* next_server;

int main(int, char const *[]);

void tree_server_close(int signum);

#endif

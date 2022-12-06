#ifndef _TREE_SERVER_PRIVATE_H
#define _TREE_SERVER_PRIVATE_H

#include "client_zookeeper-private.h"

extern const char* zook_address_port;
extern const char* server_port;

zhandle_t* zh;

int main(int, char const *[]);

void tree_server_close(int signum);
#endif

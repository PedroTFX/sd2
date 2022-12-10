#ifndef _TREE_CLIENT_PRIVATE_H
#define _TREE_CLIENT_PRIVATE_H

#include "client_zookeeper-private.h"

void select_head_and_tail_servers(zoo_string* children_list, char* root_path, zhandle_t* zh);

void showMenu();

void readOption(char* input, int size);

void executeCommand(char* option);

int commandIsPut(char* option);

int commandIsGet(char* option);

int commandIsDel(char* option);

int commandIsSize(char* option);

int commandIsHeight(char* option);

int commandIsGetKeys(char* option);

int commandIsGetValues(char* option);

int commandIsRandom(char*option);

int commandIsVerify(char* option);

void executePut(char* option);

void executeGet(char* option);

void executeDel(char* option);

void executeSize();

void executeHeight();

void executeGetKeys();

void executeGetValues();

void executeVerify(char* option);

void sig_pipe_handler(int signal);

#endif

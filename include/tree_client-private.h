void showMenu();

void readOption(char* input, int size);

int commandIsPut(char* option);

int commandIsGet(char* option);

void executePut(struct rtree_t* r_tree, char* option);

void executeGet(struct rtree_t* r_tree, char* option);

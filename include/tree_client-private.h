void showMenu();

void readOption(char* input, int size);

int commandIsPut(char* option);

int commandIsGet(char* option);

int commandIsDel(char* option);

int commandIsGetKeys(char* option);

void executePut(struct rtree_t* r_tree, char* option);

void executeGet(struct rtree_t* r_tree, char* option);

void executeDel(struct rtree_t* r_tree, char* option);

void executeGetKeys(struct rtree_t* rtree);

void showMenu();

void readOption(char* input, int size);

int commandIsPut(char* option);

int commandIsGet(char* option);

int commandIsDel(char* option);

int commandIsSize(char* option);

int commandIsHeight(char* option);

int commandIsGetKeys(char* option);

int commandIsGetValues(char* option);

void executePut(struct rtree_t* r_tree, char* option);

void executeGet(struct rtree_t* r_tree, char* option);

void executeDel(struct rtree_t* r_tree, char* option);

void executeSize(struct rtree_t* r_tree, char* option);

void executeHeight(struct rtree_t* r_tree, char* option);

void executeGetKeys(struct rtree_t* rtree);

void executeGetValues(struct rtree_t* rtree);

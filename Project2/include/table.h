
typedef struct _Table {
	int tid;
	char name[TABLENAME];
} Table;

typedef struct _TablePool {
	int count;
	char tids[NTID + 1];
	Table tables[NTABLE];
} TablePool;

void init_tablepool();
int open_table(char*);
int close_table(int);
void print_tp();


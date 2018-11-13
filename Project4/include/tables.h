
typedef struct _Table {
	utable_t tid;
	int fd;
	int num_col;
	char name[TABLENAME];
} Table;

typedef struct _TablePool {
	int count;
	char tids[NTID + 1];
	Table tables[NTABLE];
} TablePool;

void init_tablepool();
utable_t open_table(char*, unumber_t);
int close_table(utable_t);
void print_tp();
char* get_path(utable_t);
int get_fd(utable_t);
void put_fd(utable_t, int);
void close_tables();

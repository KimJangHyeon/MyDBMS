
typedef struct _Table {
	utable_t tid;
	int fd;
	char name[TABLENAME];
	pthread_mutex_t lock;
	bool extend;
} Table;

typedef struct _TablePool {
	int count;
	char tids[NTID + 1];
	Table tables[NTABLE];
} TablePool;

bool get_isExtend(utable_t);
bool extend_lock(utable_t);
void extend_release(utable_t);
int disk_lock(utable_t);
int disk_release(utable_t);
void init_tablepool();
utable_t open_table(char*);
int close_table(utable_t);
void print_tp();
char* get_path(utable_t);
int get_fd(utable_t);
void put_fd(utable_t, int);


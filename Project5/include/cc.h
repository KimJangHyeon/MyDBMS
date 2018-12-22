enum lock_mode {
	SHARED, EXCLUSIVE
};

enum trx_state {
	IDLE, RUNNING, WAITING
};


typedef struct _lock_t lock_t;
typedef struct _trx_t {
	int trx_id;
	enum trx_state state;
	list<lock_t*> trx_locks;
	lock_t* wait_lock;
} trx_t;

typedef struct _lock_t {
	utable_t tid;
	ukey64_t key;
	enum lock_mode mode;
	trx_t* trx;	
} lock_t;


int insert(utable_t, ukey64_t, udata_t[]);
int erase(utable_t, ukey64_t);
udata_t* find(utable_t, ukey64_t, int, int*);
int update(utable_t, ukey64_t, udata_t[], int, int*);
void scan_table(JoinData&);

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

typedef struct _trx_arr_t {
	int g_trx;
	trx_t trxs[MAXTXN];
} trx_arr_t;

typedef struct _page_hash_node {
	lock_t* tail;
	lock_t* head;
	unumber_t key;
	struct _page_hash_node * next;
} page_hash_node;

typedef struct _page_hash_t {
	list<page_hash_node> hash_list;
} page_hash_t;

class CC {
	private:
		pthread_spinlock_t latch;
		trx_arr_t trx_arr;
		page_hash_t phash;
		void free_trx(trx_t * trx);

	public:
		void global_latch();
		int get_trx();
		char marking_lock(int txn_id, unumber_t h_key);	//return running or sleep
		char deadlock_detection(int txn_id);	//return FAILED, SUCCESS
		
		void global_release();
};


int insert(utable_t, ukey64_t, udata_t[]);
int erase(utable_t, ukey64_t);
udata_t* find(utable_t, ukey64_t, int, int*);
int update(utable_t, ukey64_t, udata_t[], int, int*);
void scan_table(JoinData&);

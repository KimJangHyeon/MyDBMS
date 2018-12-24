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

	pthread_cond_t cond;
	pthread_mutex_t mutex;
} trx_t;

typedef struct _lock_t {
	utable_t tid;
	ukey64_t key;
	enum lock_mode mode;
	udata_t old_value[15];
	udata_t new_value[15];
	trx_t* trx;
	struct _lock_t* link;
} lock_t;

typedef struct _trx_arr_t {
	int g_trx;
	trx_t trxs[MAXTXN];
} trx_arr_t;

typedef struct _page_hash_node {
	lock_t* tail;
	lock_t* head;
	unumber_t key;
} page_hash_node;


class CC {
	private:
		pthread_spinlock_t latch;
		trx_arr_t trx_arr;
		vector<page_hash_node> phash;

		bool not_head_check_is_runnable(int txn_id, lock_t* head, lock_t* lock);
		void free_trx(trx_t * trx);
		unumber_t find_phash(unumber_t h_key, bool & isSuccess);
		
	public:
		CC();
		trx_t* find_trx_pointer(int trx_id);	
		void global_latch();
		int get_trx();
		void rm_trx(int txn_id);
		bool marking_lock(int txn_id, unumber_t h_key, lock_t* lock);	//return running or sleep
		bool deadlock_detection(int txn_id);	//return FAILED, SUCCESS
		void do_undo();	
		void global_release();
};


int insert(utable_t, ukey64_t, udata_t[]);
int erase(utable_t, ukey64_t);
udata_t* find(utable_t, ukey64_t, int, int*);
int update(utable_t, ukey64_t, udata_t[], int, int*);
void scan_table(JoinData&);

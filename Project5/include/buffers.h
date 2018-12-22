
/*
   state(Prepare) --> 
		*/

typedef enum _State {
	Empty, Prepare, Loading, Running, Cleaning
} State;

typedef struct _ControlBlock {
	pthread_rwlock_t latch;
	State state;
	utable_t tid;
	uoffset_t off;
	int pin;
	bool isDirty;
	int lru_next;
	int lru_prev;
} ControlBlock;

typedef struct _Buffer {
	ControlBlock cb;
	Page* frame;
} Buffer;


typedef struct _BufferPool {
	IndexQueue* queue;
	pthread_spinlock_t cp_latch;
	pthread_spinlock_t lru_latch;
	int num_buf;
	int victim_index;
	int latest_index;
	Buffer* buffers;
} BufferPool;

int init_db(int);
void read_buffer(utable_t, uoffset_t, Page*);
void write_buffer(utable_t, uoffset_t, Page*);
void dealloc_buffer(utable_t, uoffset_t);
void evict_tid_buffer(utable_t);
int shutdown_db(void);
void debug_lru();

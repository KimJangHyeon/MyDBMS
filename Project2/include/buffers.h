
typedef enum _State {
	Empty, Prepare, Loading, Running, Cleaning
} State;

typedef struct _ControlBlock {
	State state;
	RWLock rw;
	utable_t tid;
	uoffset_t off;
	bool isDirty;
	int lru_next;
	int lru_prev;
} ControlBlock;

typedef struct _Buffer {
	ControlBlock cb;
	Page* frame;
} Buffer;


typedef struct _BufferPool {
	pthread_spinlock_t index_lock;
	IndexQueue* queue;
	int num_buf;
	int victim_index;
	int latest_index;
	Buffer* buffers;
} BufferPool;

int init_db(int);
void read_buffer(utable_t, uoffset_t, Page*);
void write_buffer(utable_t, uoffset_t, Page*);

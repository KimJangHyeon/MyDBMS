
typedef struct _BufferPool {
	int clock;
	IndexQueue* queue;
	int num_buf;
	Buffer* buffers;
} BufferPool;

enum State {
	Init, Empty, Loading, Running, Cleaning
};

typedef _FLock {
	State state;
	utable_t fetch_tid;
	uoffset_t fetch_off;
} FLock;

typedef _ControlBlock {
	utable_t tid;
	uoffset_t off;
	bool isDirty;
	RWLock pin;
	bool ref;
} ControlBlock;

typedef struct _Buffer {
	FLock f_lock;
	ControlBlock cb;
	Page* frame;
} Buffer;

void read_buffer(utable_t, uoffset_t, Page*);
void write_buffer(utable_t, uoffset_t, Page*);

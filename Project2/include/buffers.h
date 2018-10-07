
enum State {
	Init, Empty, Loading, Running, Cleaning
};

typedef struct _FLock {
	enum State state;
	utable_t fetch_tid;
	uoffset_t fetch_off;
} FLock;

typedef struct _ControlBlock {
	utable_t tid;
	uoffset_t off;
	bool isDirty;
	int pin;
	RWLock rw;
	bool ref;
} ControlBlock;

typedef struct _Buffer {
	FLock f_lock;
	ControlBlock cb;
	Page* frame;
} Buffer;


typedef struct _BufferPool {
	int clock;
	IndexQueue* queue;
	int num_buf;
	Buffer* buffers;
} BufferPool;

void read_buffer(utable_t, uoffset_t, Page*);
void write_buffer(utable_t, uoffset_t, Page*);

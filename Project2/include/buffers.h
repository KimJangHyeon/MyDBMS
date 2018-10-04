typedef struct _BufferPool {
	bool hit_arr[LRUCHECKGROUP];
	int last_index;
	int clock;
	int count;
	Buffer* buffers;
} BufferPool;

typedef struct _Buffer {
	utable_t tid;
	uoffset_t off;
	bool isDirty;
	RWLock pin;
	bool ref;
	Page* frame;
} Buffer;

void read_buffer(utable_t, uoffset_t, Page*);
void write_buffer(utable_t, uoffset_t, Page*);

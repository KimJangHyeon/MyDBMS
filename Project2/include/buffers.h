typedef struct _Buffer {
	utable_t tid;
	uoffset_t off;
	bool isDirty;
	RWLock pin;
	bool ref;
	Page* frame;
} Buffer;

typedef struct _BufferPool {
	bool hit_arr[LRUCHECKGROUP];
	int last_index;
	int clock;
	Buffer* bp;

} BufferPool;

void read_buffer(utable_t, uoffset_t, Page*);
void write_buffer(utable_t, uoffset_t, Page*);

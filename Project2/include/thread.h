
typedef struct _EThread {
	pthread_t threads[NETHREAD];
	pthread_mutex_t mutex[NETHREAD];
	pthread_cond_t cond;
	bool lock;
	//buf
	utable_t tids[NETHREAD];
	int sizes[NETHREAD];
} EThread;



typedef struct _ThreadPool {
	EThread ethread;
} ThreadPool;

void extend_call(utable_t, int);
void init_threads();

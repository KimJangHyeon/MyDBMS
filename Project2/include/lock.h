typedef struct _RWLock {
	int rflag;
	bool wflag;
	bool lock;
} RWLock;

void simple_lock(bool*);
void simple_release(bool*);
void read_lock(RWLock*);
void read_release(RWLock*);
void write_lock(RWLock*);
void write_release(RWLock*);
int buffer_lock(int*, int, int);

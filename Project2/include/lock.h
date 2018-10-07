typedef struct _RWLock {
	int rflag;
	bool wflag;
	bool lock;
}RWLock;

void read_lock(RWLock*);
void read_release(RWLock*);
void write_lock(RWLock*);
void write_release(RWLock*);
void buffer_lock(FLock*);
void buffer_release(FLock*);

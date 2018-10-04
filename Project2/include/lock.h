typedef struct _RWLock {
	int rflag;
	bool wflag;
	bool lock;
}RWLock;

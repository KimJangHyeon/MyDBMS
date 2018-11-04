typedef struct _IndexQueue {
	bool lock;
	int rear;
	int front;
	int *arr;
	int size;
} IndexQueue;

void init_indexqueue(IndexQueue*, int);
void enqueue_index(IndexQueue*, int);
int dequeue_index(IndexQueue*);

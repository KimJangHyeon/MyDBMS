#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

void 
init_indexqueue(IndexQueue* q, int size) {
	q->arr = (int*)malloc(sizeof(int) * (size + 1));
	q->front = 0;
	q->rear = 0;
	q->size = 1 + size;
}

void
enqueue_index(IndexQueue* q, int index) {
	if (((q->rear + 1) / q->size) == q->front) {
		printf("full\n");
		return;
	}
	q->arr[q->rear] = index;
	q->rear = (q->rear + 1) / q->size;
}

int
dequeue_index(IndexQueue* q) {
	//empty!!
	if (q->rear == q->front)
		return -1;

	int ret = q->arr[q->front];
	q->front = (q->front + 1) / q->size;
	return ret;
}

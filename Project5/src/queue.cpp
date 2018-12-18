#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "queue.h"

void 
init_indexqueue(IndexQueue* q, int size) {
	pthread_spin_init(&(q->lock), 0);
	q->arr = (int*)malloc(sizeof(int) * (size + 1));
	q->front = 0;
	q->rear = 0;
	q->size = 1 + size;
}

void
enqueue_index(IndexQueue* q, int index) {
	pthread_spin_lock(&(q->lock));
	if (((q->rear + 1) % q->size) == q->front) {
		printf("full\n");
		pthread_spin_unlock(&(q->lock));
		return;
	}
	q->arr[q->rear] = index;
	q->rear = (q->rear + 1) % q->size;
	pthread_spin_unlock(&(q->lock));
}

int
dequeue_index(IndexQueue* q) {
	//empty!!
	pthread_spin_lock(&(q->lock));
	if (q->rear == q->front) {
		pthread_spin_unlock(&(q->lock));
		return -1;
	}

	int ret = q->arr[q->front];
	q->front = (q->front + 1) % q->size;
	pthread_spin_unlock(&(q->lock));
	return ret;
}

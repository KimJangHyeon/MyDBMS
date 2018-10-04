#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lock.h"

void
read_lock(RWLock* rw) {
	if (!rw->wflag)
		__sync_val_compare_and_swap(&(rw->rflag), rw->rflag, rw->rflag + 1);
	while(__sync_val_compare_and_swap(&(rw->lock), rw->lock, 1));
}

void
read_release(RWLock* rw) {
	if(!__sync_val_compare_and_swap(&(rw->lock), rw->lock, 0)) {
		printf("err:(read_release): release without locking\n");
		exit(0);
	}
	if (!__sync_val_compare_and_swap(&(rw->rflag), rw->rflag, rw->rflag - 1)) {
		printf("err:(read_release): rflag (-) value\n");
		exit(0);
	}
}

void 
write_lock(RWLock* rw) {
	while (__sync_val_compare_and_swap(&(rw->lock), rw->lock, 1));
	while (rw->rflag != 0);
	while (__sync_val_compare_and_swap(&(rw->lock), rw->lock, 1)) {
		printf("WARN(write lock): rflag 0 but lock none 0 warning\n");
	}
}

void
write_release(RWLock* rw) {
	if (!__sync_val_compare_and_swap(&(rw->lock), rw->lock, 0)) {
		printf("ERR:(write_release): relase without locking\n");
		exit(0);
	}
	if (1 != __sync_val_compare_and_swap(&(rw->wflag), rw->wflag, 0)) {
		printf("ERR:(write_release): release wflag is not 1\n");
		exit(0);
	}
}

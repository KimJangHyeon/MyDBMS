#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "params.h"
#include "types.h"
#include "lock.h"
#include "pages.h"
#include "disks.h"
#include "thread.h"

extern ThreadPool tp;

void
init_threads() {
	for (int i = 0; i < NETHREAD; i++) {
		tp.ethread.tids[i] = 0;
		tp.ethread.sizes[i] = -1;
		tp.ethread.mutex[i] = PTHREAD_MUTEX_INITIALIZER;
	}
	tp.ethread.cond = PTHREAD_COND_INITIALIZER;

	for (int i = 0; i < NETHREAD; i++) {
		pthread_create(&tp.ethread.threads[i], NULL, extend_page, (void*)i);
	}
}

void 
extend_call(utable_t tid, int sz) {
	int index = -1;
	while(!__sync_bool_compare_and_swap(&(tp.ethread.lock), 0, 1));
	
	while (1) {
		for (int i = 0; i < NETHREAD; i++) {
			//find
			if (tp.ethread.sizes[i] == -1) {
				tp.ethread.tids[i] = tid;
				tp.ethread.sizes[i] = sz;
				index = i;
				break;
			}
		}
		if (index != -1) 
			break;
	}
	
	//signal to index
	pthread_cond_signal(&(tp.ethread.cond));

	if (!__sync_bool_compare_and_swap(&(tp.ethread.lock), 1, 0)) {
		printf("extend call err!!\n");
	}

}	

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "params.h"
#include "types.h"
#include "lock.h"
#include "pages.h"
#include "disks.h"
#include "tables.h"
#include "thread.h"

extern ThreadPool thp;
void 
extend_call(utable_t tid, int sz) {
	int index = -1;
	printf("start1\n");
	while(!__sync_bool_compare_and_swap(&(thp.ethread.lock), 0, 1));
	
	if (get_isExtend(tid)) {
		if(!__sync_bool_compare_and_swap(&(thp.ethread.lock), 1, 0)) 
			printf("extend caall err!!\n");
		return;
	}
	printf("start2\n");
	

	while (1) {
		for (int i = 0; i < NETHREAD; i++) {
			//find
			if (thp.ethread.sizes[i] == -1) {
				thp.ethread.tids[i] = tid;
				thp.ethread.sizes[i] = sz;
				index = i;
				break;
			}
		}
		if (index != -1) 
			break;
	}
	
	printf("signal!!(%d)\n", index);
	//signal to index
	pthread_cond_signal(&(thp.ethread.cond[index]));

	if (!__sync_bool_compare_and_swap(&(thp.ethread.lock), 1, 0)) {
		printf("extend call err!!\n");
	}

}	

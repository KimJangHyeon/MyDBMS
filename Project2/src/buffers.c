#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include "types.h"
#include "params.h"
#include "pages.h"
#include "lock.h"
#include "disks.h"
#include "queue.h"
#include "buffers.h"
#include "tables.h"
#include "inits.h"
BufferPool * bp;

int
init_db (int num_buf) {
	bp = (BufferPool*)malloc(sizeof(BufferPool));
	bp->num_buf = num_buf;
	bp->queue = (IndexQueue*)malloc(sizeof(IndexQueue));
	bp->buffers = (Buffer*)malloc(sizeof(Buffer) * num_buf);
	pthread_spin_init(&(bp->index_lock), 0);

	//do spin lock
	pthread_spin_lock(&(bp->index_lock));
	
	bp->victim_index = -1;
	bp->latest_index = -1; 
	
	// release spin lock
	pthread_spin_unlock(&(bp->index_lock));

	//========init bp done ===============
	init_tablepool();
	init_indexqueue(bp->queue, num_buf);
	for (int i = 0; i < num_buf; i++) {
		memset(&(bp->buffers[i]), 0, sizeof(Buffer));
		bp->buffers[i].cb.state = Empty;
		enqueue_index(bp->queue, i);
	}

	return 0;	
}

/*
		if (bp->latest_index != -1) {
			bp->buffers[bp->latest_index].cb.lru_prev = i;
			bp->buffers[i].cb.lru_next = bp->latest_index;
			bp->latest_index = i;

		}
		if (bp->victim_index == -1) {
			bp->victim_index = i;
		}
		bp->latest_index = i;
*/


//check after pin++, if state != Running if so, find (tid, off)
void
read_buffer(utable_t tid, uoffset_t offset, Page* page) {
	//if has tid and offset==> (problem checked but it became targeted)

	for (int i = 0; i < bp->buffers[i]; i++) {
		
	}

	load_page(tid, offset, page);
}

void
write_buffer(utable_t tid, uoffset_t offset, Page* page) {
	flush_page(tid, offset, page);
}

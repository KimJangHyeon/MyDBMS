#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include "types.h"
#include "params.h"
#include "pages.h"
#include "disks.h"
#include "queue.h"
#include "buffers.h"
#include "tables.h"
#include "inits.h"
#include "utils.h"

BufferPool * bp;

int
init_db (int num_buf) {
	bp = (BufferPool*)malloc(sizeof(BufferPool));
	bp->num_buf = num_buf;

	bp->queue = (IndexQueue*)malloc(sizeof(IndexQueue));
	bp->buffers = (Buffer*)malloc(sizeof(Buffer) * num_buf);

	bp->victim_index = -1;
	bp->latest_index = -1; 
	

	//========init bp done ===============
	init_tablepool();
	init_indexqueue(bp->queue, num_buf);
	
	for (int i = 0; i < num_buf; i++) {
		memset(&(bp->buffers[i]), 0, sizeof(Buffer));
		bp->buffers[i].frame = (Page*)malloc(sizeof(Page));
		bp->buffers[i].cb.state = Empty;
		bp->buffers[i].cb.lru_next = -1;
		bp->buffers[i].cb.lru_prev = -1;
		enqueue_index(bp->queue, i);
	}

	return 0;	
}

int 
find_buffer(utable_t tid, uoffset_t offset) {
	for (int i = 0; i < bp->num_buf; i++) {
		if (bp->buffers[i].cb.off == offset && bp->buffers[i].cb.tid == tid) {
			return i;
		}
	}
	return -1;
}

int
find_lru() {
	int index;
	while (1) {
		index = bp->victim_index;
		if (index == -1) {
			printf("(find lru -1 error!!)\n");
			exit(0);
		}
		while(index != -1) {
			if (bp->buffers[index].cb.pin == 0) 
				return index;
			else 
				index = bp->buffers[index].cb.lru_prev;
		}
	}
}

void
lru_change(int index) {
	int next_index, prev_index;
	int cur_latest_index = bp->latest_index;
	next_index = bp->buffers[index].cb.lru_next;
	prev_index = bp->buffers[index].cb.lru_prev;
	printf("index: %d\n", index);	
	if (cur_latest_index != index) {
		if (prev_index != -1) {
			bp->buffers[prev_index].cb.lru_next = next_index;
		}

		if (next_index != -1) {
			bp->buffers[next_index].cb.lru_prev = prev_index; 
		}

		//=========== side lru change done 
	

		//lock
		bp->latest_index = index;
		if (bp->victim_index == index && prev_index != -1) {
			bp->victim_index = prev_index;
		}
	
		if (bp->victim_index == -1) {
			bp->victim_index = index;
		}
		bp->buffers[cur_latest_index].cb.lru_prev = index;

		bp->buffers[index].cb.lru_next = cur_latest_index;
		bp->buffers[index].cb.lru_prev = -1;
	}

}

void 
lru_clean(int index) {
	int next_index, prev_index;	
	int cur_latest_index, cur_victim_index;
	char isLatest = 0;
	char isVictim = 0;

	next_index = bp->buffers[index].cb.lru_next;
	prev_index = bp->buffers[index].cb.lru_prev;
	cur_latest_index = bp->latest_index;
	cur_victim_index = bp->victim_index;

	if (prev_index != -1) {
		if (cur_victim_index == index) {
			isVictim = 1;
			bp->victim_index = prev_index;
		}
		bp->buffers[prev_index].cb.lru_next = next_index;
	}

	if (next_index != -1) {
		if (cur_latest_index == index) {
			isLatest = 1;
			bp->latest_index = next_index;
		}
		bp->buffers[next_index].cb.lru_prev = prev_index;
	}

	if (cur_latest_index == index && (!isLatest)) {
		bp->latest_index = -1;
	}
	if (cur_victim_index == index && (!isVictim)) {
		bp->victim_index = -1;
	}

}

void
clean_buffer(int index) {
	if (bp->buffers[index].cb.isDirty) {
		flush_page(bp->buffers[index].cb.tid, bp->buffers[index].cb.off, bp->buffers[index].frame);
		bp->buffers[index].cb.isDirty = 0;
	}

	if (bp->buffers[index].cb.pin != 0) {
		printf("clean buffer pin not 0 err!!\n");
		exit(0);
	}
	bp->buffers[index].cb.tid = 0;
	bp->buffers[index].cb.off = 0;
}	

int
evict_buffer() {
	printf("EVICT!!!\n\n");
	int target_index;
	//===================== lru changed

	target_index = find_lru();
	
	if (bp->buffers[target_index].cb.state == Running) 
		bp->buffers[target_index].cb.state = Cleaning;
	else {
		printf("(evict_buffer err!!): Running -> Cleaning\n");
		exit(0);
	}

	
	clean_buffer(target_index);
	
	if (bp->buffers[target_index].cb.state == Cleaning) 
		bp->buffers[target_index].cb.state = Empty;
	else {
		printf("(evict_buffer err!!): Cleaning -> Empty\n");
		exit(0);
	}

	return target_index;
}

void
evict_tid_buffer(utable_t tid) {
	char isAllClean = 0;
	int index, prev_index;
	while(!isAllClean) {
		printf("first while\n");
		isAllClean = 1;
		index = bp->victim_index;
		while (index != -1) {
			printf("index: %d\n", index);
			printf("prev: %d\n", prev_index);
			prev_index = bp->buffers[index].cb.lru_prev;
			if (bp->buffers[index].cb.tid != tid) {
				index = prev_index;
				continue;
			}

			isAllClean = 0;
			if (bp->buffers[index].cb.state == Running) 
			bp->buffers[index].cb.state = Cleaning;
			else {
				printf("(evict_buffer err!!): Running -> Cleaning\n");
				exit(0);
			}

			clean_buffer(index);
			lru_clean(index);
			if (bp->buffers[index].cb.state == Cleaning) 
				bp->buffers[index].cb.state = Empty;
			else {
				printf("err: dealloc Cleaning->Empty\n");
			}

			enqueue_index(bp->queue, index);
			index = prev_index;
		}
	}
}

int 
access_buffer(utable_t tid, uoffset_t offset) {
	//if has tid and offset==> (problem checked but it became targeted)
	int index = find_buffer(tid, offset);
	printf("access buffer\n");
	//load page
	if (index == -1) {
		index = dequeue_index(bp->queue);

		//is no empty buffer(do evict buffer)
		if (index == -1) 
			index = evict_buffer();

		//--------index is empty
		if (bp->buffers[index].cb.state == Empty) 
			bp->buffers[index].cb.state = Prepare;

		bp->buffers[index].cb.tid = tid;
		bp->buffers[index].cb.off = offset;

		if (bp->buffers[index].cb.state == Prepare) 
			bp->buffers[index].cb.state = Loading;
		
		printf("tid: %ld, o: %ld\n", tid, offset);
		load_page(tid, offset, bp->buffers[index].frame);

		if (bp->buffers[index].cb.state == Loading) 
			bp->buffers[index].cb.state = Running;
	}
	lru_change(index);
	d_print_lru_priority(bp);

	return index;

}

int
try_empty_buffer(utable_t tid, uoffset_t offset) {
	int index = find_buffer(tid, offset);
	
	if (index == -1)
		return index;

	if (bp->buffers[index].cb.pin != 0) {
		printf("err: dealloc target pin is not 0!!\n");
		exit(0);
	}
	
	if (bp->buffers[index].cb.state == Running) 
		bp->buffers[index].cb.state = Cleaning;

	else {
		printf("err: dealloc Running->Cleaning\n");
	}
	clean_buffer(index);
	bp->buffers[index].cb.tid = 0;
	bp->buffers[index].cb.off = 0;
	lru_clean(index);
	if (bp->buffers[index].cb.state == Cleaning) 
		bp->buffers[index].cb.state = Empty;
	else {
		printf("err: dealloc Cleaning->Empty\n");
	}

	return index;
}

//check after pin++, if state != Running if so, find (tid, off)
void
read_buffer(utable_t tid, uoffset_t offset, Page* page) {
	d_print_buffer_hpage(bp, tid, offset);
	printf("read buffer(%ld, %ld)\n", tid, offset);
	int index = access_buffer(tid, offset);
	bp->buffers[index].cb.pin++;
	memcpy(page, bp->buffers[index].frame, PAGESIZE);
	bp->buffers[index].cb.pin--;
}

void
write_buffer(utable_t tid, uoffset_t offset, Page* page) {
	printf("write buffer(%ld, %ld)\n", tid, offset);
	int index = access_buffer(tid, offset);
	bp->buffers[index].cb.pin++;
	if (!bp->buffers[index].cb.isDirty)
		bp->buffers[index].cb.isDirty = 1;
	memcpy(bp->buffers[index].frame, page, PAGESIZE);
	bp->buffers[index].cb.pin--;
	//flush_page(tid, offset, page);
}

void
dealloc_buffer(utable_t tid, uoffset_t offset) {
	printf("dealloc_buffer(%ld, %ld)\n", tid, offset);
	int index = try_empty_buffer(tid, offset);
	
	if (index != -1)
		enqueue_index(bp->queue, index);

	dealloc_page(tid, offset);
}





void 
debug_lru() {
	d_print_lru_priority(bp);
}

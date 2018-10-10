#include <stdio.h>
#include <stdbool.h>
#include "types.h"
#include "params.h"
#include "pages.h"
#include "lock.h"
#include "disks.h"
#include "queue.h"
#include "buffers.h"

BufferPool * bp;

int
init_db (int num_buf) {
	bp = (BufferPool*)malloc(sizeof(BufferPool));
	bp->num_buf = num_buf;
	bp->clock = 0;
	bp->queue = (IndexQueue*)malloc(sizeof(IndexQueue));
	init_indexqueue(qp->queue, num_buf);
	bp->buffers = (Buffer*)malloc(sizeof(Buffer) * num_buf);

	//========init bp done ===============
	for (int i = 0; i < num_buf; i++) {

	}
}

void
read_buffer(utable_t tid, uoffset_t offset, Page* page) {
	load_page(tid, offset, page);
}

void
write_buffer(utable_t tid, uoffset_t offset, Page* page) {
	flush_page(tid, offset, page);
}

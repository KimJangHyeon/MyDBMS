
#include "types.h"
#include "params.h"
#include "pages.h"
#include "lock.h"
#include "disks.h"
#include "buffers.h"

BufferPool* bp;

int 
init_db(int num_buf) {
	bp = (BufferPool*)malloc(sizeof(BufferPool));
	memset(bp, 0, sizeof(BufferPool));
	bp->buffers = (Buffer*)malloc(sizeof(Buffer) * num_buf);
	for (int i = 0; i < num_buf; i++) 
		memset(&(bp->buffers[i]), 0, sizeof(Buffer));

}

void
read_buffer(utable_t tid, uoffset_t offset, Page* page) {
	load_page(tid, offset, page);
}

void
write_buffer(utable_t tid, uoffset_t offset, Page* page) {
	flush_page(tid, offset, page);
}



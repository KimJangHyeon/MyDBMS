#include <stdio.h>
#include <stdbool.h>
#include "types.h"
#include "params.h"
#include "pages.h"
#include "disks.h"
#include "buffers.h"

void
read_buffer(utable_t tid, uoffset_t offset, Page* page, bool isForWrite) {
	if(isForWrite) {
		printf("%lu is for write\n", offset);
	} else {
		printf("%lu is for read\n", offset);
	}
	load_page(tid, offset, page);
}

void
write_buffer(utable_t tid, uoffset_t offset, Page* page) {
	flush_page(tid, offset, page);
}

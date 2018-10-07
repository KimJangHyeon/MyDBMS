#include <stdio.h>
#include <stdbool.h>
#include "types.h"
#include "params.h"
#include "pages.h"
#include "disks.h"
#include "buffers.h"

void
read_buffer(utable_t tid, uoffset_t offset, Page* page) {
	load_page(tid, offset, page);
}

void
write_buffer(utable_t tid, uoffset_t offset, Page* page) {
	flush_page(tid, offset, page);
}

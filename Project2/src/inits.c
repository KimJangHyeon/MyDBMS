#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "params.h"
#include "lock.h"
#include "queue.h"
#include "pages.h"
#include "disks.h"
#include "buffers.h"
#include "utils.h"
#include "inits.h"
//called open_table
void
init_table(utable_t tid) {
	HeaderPage hp;
	memset(&hp, 0, sizeof(HeaderPage)); 
	hp.f_page_offset = 0;
	hp.r_page_offset = 0;
	hp.number_of_pages = 1;
	hp.number_of_free_pages = 0;
	flush_page(tid, HEADEROFFSET, (Page*)&hp);
	d_print_dpage(tid, 0, DHEADER);
	extend_page(tid, 7);
}

uoffset_t
init_root(utable_t tid, bool isLeaf) {
    NodePage node;
    HeaderPage* hp = (HeaderPage*)malloc(sizeof(HeaderPage));
    uoffset_t offset;
    memset(&node, 0, PAGESIZE);
    node.header_top.poffset = 0;
    node.header_top.isLeaf = isLeaf;
    node.header_top.num_keys = 0;


    offset = alloc_page(tid);
    flush_page(tid, offset, (Page*)&node);

    read_buffer(tid, HEADEROFFSET, (Page*)hp);
    hp->r_page_offset = offset;
    write_buffer(tid, HEADEROFFSET, (Page*)hp);
    return offset;
}



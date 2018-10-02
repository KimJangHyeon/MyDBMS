#include <string.h>

#include "types.h"
#include "params.h"
#include "pages.h"
#include "disks.h"

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
	extend_page(tid, 7);

}


#include "pages.h"

//called open_table
init_table(utable_t tid) {
	HeaderPage hp;
	memset(&hp, 0, sizeof(HeaderPage)); 
	hp.f_page_offset = 0;
	hp.r_page_offset = 0;
	hp.number_of_pages = 1;
	hp.number_of_free_pages = 0;
	flush_page(tid, offset, (Page*)&hp);
	expand_page(tid, FREEINIT);

}

#define DHEADER		0
#define DFREE		1
#define DNODE		2

void panic (char*);
void d_print_dpage(utable_t, uoffset_t, int);
void  d_print_mpage(utable_t, Page*, int);
void d_free_page_ditector(utable_t);
void d_print_tree(utable_t);




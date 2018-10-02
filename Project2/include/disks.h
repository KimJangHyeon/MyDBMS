int open_disk(int tid);
int close_disk(int tid);
void flush_page(utable_t, uoffset_t, Page*);
void load_page(utable_t, uoffset_t, Page*);
uoffset_t alloc_page(utable_t);
void dealloc_page(utable_t, uoffset_t);
void extend_page(utable_t, int);

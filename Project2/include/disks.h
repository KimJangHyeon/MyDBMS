int open_disk(utable_t tid);
int close_disk(utable_t tid);
unumber_t disk_size(utable_t);
void flush_page(utable_t, uoffset_t, Page*);
void load_page(utable_t, uoffset_t, Page*);
uoffset_t alloc_page(utable_t);
void dealloc_page(utable_t, uoffset_t);
void extend_page(utable_t, int);

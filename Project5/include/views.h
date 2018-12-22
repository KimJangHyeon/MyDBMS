uoffset_t find_leaf(utable_t, ukey64_t, LeafPage*);
udata_t* do_find(utable_t, ukey64_t);
void do_insert(utable_t, ukey64_t, udata_t[]);
void do_erase(utable_t tid, ukey64_t key);
void do_update(utable_t, ukey64_t, udata_t[], int* result);
void do_scan_table(JoinData&);

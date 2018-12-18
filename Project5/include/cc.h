int insert(utable_t, ukey64_t, udata_t[]);
int erase(utable_t, ukey64_t);
udata_t* find(utable_t, ukey64_t, int, int*);
int update(utable_t, ukey64_t, udata_t[], int, int*);
void scan_table(JoinData&);

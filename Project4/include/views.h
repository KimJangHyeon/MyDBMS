udata_t* find(utable_t, ukey64_t);
void insert(utable_t, ukey64_t, udata_t[]);
void erase(utable_t tid, ukey64_t key);
unumber_t scan_table(utable_t tid, std::vector<ColInfo>* v, JoinData*);

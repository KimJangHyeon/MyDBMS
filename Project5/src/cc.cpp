#include <stdio.h>
#include <vector>

#include "params.h"
#include "types.h"
#include "pages.h"
#include "join_struct.h"
#include "views.h"
#include "cc.h"

int
insert(utable_t tid, ukey64_t key, udata_t value[]) {
	do_insert(tid, key, value);
	return 0; 
}

int 
erase(utable_t tid, ukey64_t key) {
	do_erase(tid, key);
	return 0;
}

udata_t*
find(utable_t tid, ukey64_t key, int txn_id, int* result) {
	return do_find(tid, key);
}

int
update(utable_t tid, ukey64_t key, udata_t value[], int txn_id, int* result) {
	do_update(tid, key, value, result);
	return 0;
}

void
scan_table(JoinData& join_data) {
	do_scan_table(join_data);
}

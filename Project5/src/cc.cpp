#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <vector>

using namespace std;
#include "params.h"
#include "types.h"
#include "pages.h"
#include "join_struct.h"
#include "views.h"
#include "cc.h"

unumber_t 
get_pid(utable_t tid, ukey64_t key) {
	uoffset_t offset;
	LeafPage* leaf_page = (LeafPage*)malloc(sizeof(LeafPage));
	offset = find_leaf(tid, key, leaf_page);

	return (offset + tid);
}

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

	//get pid
	unumber_t h_key = get_pid(tid, key);

	//latch

	//marking_lock
		//return running or sleep?

	//deadlock_detection
		//undo

	//release_latch

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

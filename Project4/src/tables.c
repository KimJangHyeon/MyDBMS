#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "params.h"
#include "types.h"
#include "pages.h"
#include "tables.h"
#include "inits.h"
#include "disks.h"
#include "queue.h"
#include "buffers.h"


TablePool tp;

void 
init_tablepool() {
	tp.count = 0;
	memset (tp.tables, 0, NTABLE * sizeof(Table));
	memset (tp.tids, 0, (NTID + 1) * sizeof(char));
}

utable_t
get_tid() {
	//tids lock
	for (int i = 1; i < NTID + 1; i++) {
		if (tp.tids[i] == 0) {
			tp.tids[i] = 1;
			return i;
		}
		else {
		}
	}
	return TIDFULL;
}

char*
get_path(utable_t tid) {
	for (int i = 0; i < tp.count; i++) {
		if (tp.tables[i].tid == tid) 
			return tp.tables[i].name;
			//memcpy(path, tp.tables[i].name, TABLENAME);
	}
}

int
rm_tid(utable_t tid) {
	//tids lock
	if (tid <= 0 || tid > NTID) {
		return -1;
	}
	tp.tids[tid] = 0;
	return 0;
}

char
tid_exist(utable_t tid) {
	return tp.tids[tid];
}

void
put_fd(utable_t tid, int fd) {
	for (int i = 0; i < tp.count; i++) {
		if (tp.tables[i].tid == tid) {
			tp.tables[i].fd = fd;
			return;
		}
	}	
}

int 
get_fd(utable_t tid) {
	for (int i = 0; i < tp.count; i++) { 
		if (tp.tables[i].tid == tid) 
			return tp.tables[i].fd;
	}
}

put_col(utable_t tid, unumber_t num_col) {
	for (int i = 0; i < tp.count; i++) {
		if (tp.tables[i].tid == tid) {
			tp.tables[i].num_col = num_col;
			return;
		}
	}
}

get_col(utable_t tid) {
	for (int i = 0; i < tp.count; i++) { 
		if (tp.tables[i].tid == tid) 
			return tp.tables[i].num_col;
	}
}

//case --> path & num_col do not match 
utable_t 
open_table(char* path, unumber_t num_col) {
	int mid;
	int high = tp.count - 1;
	int low = 0;
	utable_t tid;
	unumber_t size;
	int compare;
	int fd;
	char* dir = "datas/";
	HeaderPage hp;

	Table* temp = (Table*)malloc(sizeof(Table));

	//open_disk
	//something
	if ((tid = get_tid()) == TIDFULL) {
		// table add deny
		return -1;
	}
	
	memset(temp, 0, sizeof(Table));
	memcpy(temp->name, dir, strlen(dir));
	memcpy(temp->name + strlen(dir), path, sizeof(char) * strlen(path));
	temp->tid = tid;
	temp->fd = FDCLOSE;
	temp->num_col = 0;

	if (high == -1) {
		memcpy (&(tp.tables[0]), temp, sizeof(Table));
		tp.count++;
	
		//for init_table
		open_disk(tid);
		size = disk_size(tid);
		close_disk(tid);
		if (size == 0) { 
			init_table(tid, num_col);
			put_col(tid, num_col);
		}
		else {
			//check header->num_col == num_col
			load_page(tid, 0, (Page*)&hp);
			put_col(tid, hp.num_col);
		}
		return 1;
	}
	while (low <= high) {
		mid = (high + low) / 2;
		compare = strcmp (tp.tables[mid].name + 6, path);
		if ((low == high) && (compare != 0)) {
			break;
		}
		
		if (compare > 0) 
			high = mid - 1;
		else if (compare < 0)
			low = mid + 1;
		else {
			return 0;
		}
	}


	if (compare < 0) {
		mid += 1;
	}
	for (int i = tp.count - 1; i >= mid; i--) {
//		memset(&(tp.tables[i + 1]), 0, sizeof(Table));
		memcpy(&(tp.tables[i + 1]), &(tp.tables[i]), sizeof(Table));
	}
//	memset(&(tp.tables[i + 1]), 0, sizeof(Table));
	memcpy(&(tp.tables[mid]), temp, sizeof(Table));
	tp.count++;
	
	//for init_table
	open_disk(tid);
	size = disk_size(tid);
	close_disk(tid);
	if (size == 0) 
		init_table(tid);
	
	return tid;	
}

int 
close_table(utable_t tid) {
	int isSuccess = 0;
	int i;

	
	if (tid_exist(tid)) {
		printf("do tid evict!!\n");
		evict_tid_buffer(tid);
	}

	if(rm_tid(tid)) {
		//no such tid
		return -1;
	}

	for (i = 0; i < tp.count; i++) {
		if (tp.tables[i].tid == tid) {
			memset (&(tp.tables[i]), 0, sizeof(Table));
			isSuccess = 1;
			break;
		}
	}
	for (int j = i; j < tp.count - 1; j++) {
		memcpy (&(tp.tables[j]), &(tp.tables[j + 1]), sizeof(Table));
	}
	if (isSuccess)	
		memset(&(tp.tables[tp.count - 1]), 0, sizeof(Table));

	if (isSuccess == 0)
		return 1;
	else { 
		tp.count--;
		debug_lru();
		return 0;
	}
}

void
close_tables() {
	for (int i = 1; i < NTID + 1; i++) {
		if (tp.tids[i] != 1)
			continue;
		close_table(i);
	}
}

void 
print_tp() {
	printf("count: %d\n", tp.count);
	printf("(");
	for (int i = 0; i < NTID + 1; i++) {
		printf("%d, ", tp.tids[i]);
	}
	printf(")\n");
	for (int i = 0; i < tp.count; i++) {
		printf("(i:%d,tid:%ld,fd:%d, p:%s), ", i, tp.tables[i].tid,tp.tables[i].fd, tp.tables[i].name);
	}
	printf("\n\n");
}

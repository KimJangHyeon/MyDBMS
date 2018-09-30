#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "param.h"
#include "table.h"

TablePool tp;

void 
init_tablepool() {
	tp.count = 0;
	memset (tp.tables, 0, NTABLE * sizeof(Table));
	memset (tp.tids, 0, (NTID + 1) * sizeof(char));
}

int
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

int
rm_tid(int tid) {
	//tids lock
	if (tid <= 0 || tid > NTID) {
		printf("tid range err\n");
		return -1;
	}
	tp.tids[tid] = 0;
	return 0;
}

int 
open_table(char* path) {
	int mid;
	int high = tp.count - 1;
	int low = 0;
	int tid;
	int compare;

	Table* temp = (Table*)malloc(sizeof(Table));

	memset(temp, 0, sizeof(Table));
	memcpy(temp->name, path, sizeof(char) * strlen(path));
	//open_disk
	//something
	if ((tid = get_tid()) == TIDFULL) {
		// table add deny
		printf("table pool is full!!\n");
		return -1;
	}
	
	temp->tid = tid;


	if (high == -1) {
		memcpy (&(tp.tables[0]), temp, sizeof(Table));
		tp.count++;
		return 1;
	}
	while (low <= high) {
		mid = (high + low) / 2;
		compare = memcmp (tp.tables[mid].name, path, TABLENAME);
		if ((low == high) && (compare != 0)) {
			break;
		}
		
		if (compare > 0) 
			high = mid - 1;
		else if (compare < 0)
			low = mid + 1;
		else {
			printf("have same path\n");
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
	
	return tid;
}

int 
close_table(int tid) {
	int isSuccess = 0;
	int i;
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
		return 0;
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
		printf("(%d, %d, %s), ", i, tp.tables[i].tid, tp.tables[i].name);
	}
	printf("\n\n");
}

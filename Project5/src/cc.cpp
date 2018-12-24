#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <list>
#include <vector>

using namespace std;
#include "params.h"
#include "types.h"
#include "pages.h"
#include "join_struct.h"
#include "views.h"
#include "cc.h"

CC cc = CC();




CC::CC() {
	pthread_spin_init(&(this->latch), 0);
	this->trx_arr.g_trx = 1;
	for (int i = 0; i < MAXTXN; i++) {
		this->trx_arr.trxs[i].trx_id = -1;
		this->trx_arr.trxs[i].state = IDLE;
		this->trx_arr.trxs[i].wait_lock = NULL;
	}

	

}

int 
CC::get_trx() {
	int get_trx = __sync_fetch_and_add(&(this->trx_arr.g_trx), 1);
	char isSuccess = 0;

	while (isSuccess == 0) {
		for (int i = 0; i < MAXTXN; i++) {
			if (__sync_bool_compare_and_swap(&(this->trx_arr.trxs[i].trx_id), -1, get_trx)) {
				isSuccess = 1;
				break;
			}
		}
	}
	return get_trx;
}

void 
CC::rm_trx(int trx_id) {
	//rm trx_arr where trx_id
	// CAS trx_id -> -1
}

void 
CC::global_latch() {
	pthread_spin_lock(&(this->latch));
}

void 
CC::global_release() {
	pthread_spin_unlock(&(this->latch));
}

bool
CC::not_head_check_is_runnable(lock_t* head, lock_t* lock) {
	lock_t* node = head;
	bool ret = 0;

	while (node != lock) {
		if (node->mode == EXCLUSIVE)
			return 0;
		if (node->mode == SHARED)
			ret = 1;
	}
	return ret;
}

unumber_t 
CC::find_phash(unumber_t h_key, bool & isSuccess) {
	page_hash_node* hash_node;
	//unumber_t index;

	for (unumber_t i = 0; i < this->phash.size(); i++) {
		hash_node = &(this->phash[i]);

		if (hash_node->key > h_key) {
			isSuccess = 0;
			return i;
		}
		
		if (hash_node->key == h_key) {
			isSuccess = 1;
			return i;
		}
	}
	isSuccess = 0;
	return 0;
}

bool 
CC::marking_lock(int txn_id, unumber_t h_key, lock_t* lock) {
	bool isSuccess;
	bool isRunnable;
	unumber_t index;
	page_hash_node temp;
	page_hash_node* tphn;

	index = find_phash(h_key, isSuccess);

	if (!isSuccess) {
		temp.tail = NULL;
		temp.head = NULL;
		temp.key = h_key;
		this->phash.insert(this->phash.begin() + index, temp);
	}

	tphn = &(this->phash[index]);

	//is RUNNABLE
	if ((tphn->head == NULL) && (tphn->tail == NULL)) {
		isRunnable = 1;
		tphn->head = lock;
		tphn->tail = lock;
	} else if ((tphn->head != NULL) && (tphn->tail != NULL)) {
		tphn->tail->link = lock;
		tphn->tail = lock;
		if (lock->mode == SHARED)
			isRunnable = not_head_check_is_runnable(lock);
		else 
			isRunnable = 0;
	} else {
		//err
		fprintf(stderr, "marking lock err!! in header tail \n");
		exit(0);
	}

	// now index is valid
	return isRunnable;

}

trx_t * 
CC::find_trx_pointer(int trx_id) {
	for (int i = 0; i < MAXTXN; i++) {
		if (this->trx_arr.trxs[i].trx_id == trx_id) {
			return &(this->trx_arr.trxs[i]);
		}
	}
	return NULL;
}

unumber_t 
get_pid(utable_t tid, ukey64_t key) {
	uoffset_t offset;
	LeafPage* leaf_page = (LeafPage*)malloc(sizeof(LeafPage));
	offset = find_leaf(tid, key, leaf_page);

	return (offset + tid);
}

lock_t*
make_lock(utable_t tid, ukey64_t key, bool mode, int trx_id) {
	lock_t* ret = new lock_t;
	ret->tid = tid;
	ret->key = key;
	ret->link = NULL;
	if (mode) 
		ret->mode = EXCLUSIVE;
	else
		ret->mode = SHARED;
	ret->trx = cc.find_trx_pointer(trx_id);
	
	if (ret_trx == NULL) {
		fprintf(stderr, "make lock trx_id not found!!\n");
		exit(0);
	}
	ret->trx->trx_locks.push_back(ret);
	return ret;
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

int 
begin_tx() {
	return cc.get_trx();
}

int 
end_tx(int trx_id) {
	//release all the lock
	//wake up next lock to run

}

udata_t*
find(utable_t tid, ukey64_t key, int trx_id, int* result) {

	//get pid
	unumber_t h_key = get_pid(tid, key);

	//make lock
	lock_t* lock = make_lock(tid, key, 0, trx_id);

	//latch
	cc.global_latch();

	//marking_lock
	cc.marking_lock(trx_id, h_key, lock);
		//return running or sleep?

	//deadlock_detection
		//undo -> result = FAILED

	//if d_d_SUCCESS & m_l_SLEEP
		//release_latch
		//trans sleep

	//if d_d_success & m_l_running
		//release_latch
		//op

	//if d_d_fail 
		//undo all
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

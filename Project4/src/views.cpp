#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "params.h"
#include "types.h"
#include "pages.h"
#include "disks.h"
#include "lock.h"
#include "queue.h"
#include "buffers.h"
#include "tables.h"
#include "views.h"
#include "inits.h"
#include "utils.h"

Page* g_page;

//find
uoffset_t find_leaf(utable_t, ukey64_t, LeafPage*);

//insert
int cut(int);
void make_leaf(LeafPage*);
void make_inter(InternalPage*);
int get_left_index(uoffset_t left_offset, InternalPage* parent); 
void insert_into_leaf(utable_t tid, uoffset_t offset, unumber_t num_col, ukey64_t key, udata_t[], LeafPage* leaf);
void insert_into_new_root(utable_t, ukey64_t, ukey64_t, uoffset_t, uoffset_t, NodePage*, NodePage*);
void insert_into_node(utable_t, uoffset_t, int, ukey64_t, uoffset_t, InternalPage*);
void insert_into_node_after_splitting(int, utable_t, ukey64_t, uoffset_t, uoffset_t, InternalPage*);
void insert_into_parent(utable_t, ukey64_t, ukey64_t, uoffset_t, uoffset_t, NodePage*, NodePage*);
void insert_into_leaf_after_splitting(uoffset_t, utable_t, unumber_t num_col, ukey64_t, udata_t[], LeafPage*); 

//delete
void delete_entry(utable_t tid, uoffset_t, uoffset_t, ukey64_t, NodePage*);
void remove_entry_from_node(utable_t, uoffset_t, uoffset_t, unumber_t, ukey64_t, NodePage*);
void adjust_root(utable_t, uoffset_t, NodePage*);
int get_neighbor_index(utable_t tid, uoffset_t offset, NodePage* knode);

//join
LeafPage* get_left_leaf(utable_t tid, int cols[]);

int entry_count = 0;
int node_count = 0;
int adjust_count = 0;
int coal_count = 0;
int redis_count = 0;
	
int leaf_order = LRECORD;
int inter_order = IRECORD;

int cut(int length) {
	if (length % 2 == 0)
		return length/2;
	else 
		return length/2 + 1;
}

void 
make_inter (InternalPage* inter_page) {
	memset(inter_page, 0, PAGESIZE);
	inter_page->header_top.isLeaf = False;
	inter_page->header_top.num_keys = 0;
	inter_page->header_top.poffset = 0;
}

void
make_leaf(LeafPage* leaf) {
	memset(leaf, 0, PAGESIZE);

	leaf->header_top.poffset = 0;
	leaf->header_top.isLeaf = True;
	leaf->header_top.num_keys = 0;
	for (int i = 0; i < NMAXVAL; i++) {
		leaf->catalog.info[i].min = CATALOGINITMIN;
		leaf->catalog.info[i].max = CATALOGINITMAX;
	}
	leaf->sibling = 0;
}

int get_left_index(uoffset_t left_offset, InternalPage* parent) {
	int left_index = 0;
	while (left_index <=parent->header_top.num_keys &&
			parent->record[left_index].offset != left_offset)
		left_index++;
	return left_index;
}

//***************** find *******************
//return leaf page (state: locked)


void 
catalog_delete(LeafPage* leaf, unumber_t num_col, unumber_t num_key, ukey64_t value[NMAXVAL]) {
	udata_t min, max, old_min, old_max;
	udata_t candidate_value;
	int index;
	for (int i = 0; i < num_col - 1; i++) {
		min = leaf->catalog.info[i].max;
		old_min = leaf->catalog.info[i].min;
		max = leaf->catalog.info[i].min;
		old_max = leaf->catalog.info[i].max;
		if (old_min == value[i]) {
			for (int j = 0; j < num_key; j++) {
				candidate_value = leaf->record[j].value[i];
				if ((min > candidate_value) && (old_min != candidate_value)) {
					min = candidate_value;
				} 
			}
			leaf->catalog.info[i].min = min;
		} else if ((old_min > value[i])) {
			printf("catalog_delete: min > value err!!\n");
			exit(0);
		}

		if (old_max == value[i]) {
			for (int j = 0; j < num_key; j++) {
				candidate_value = leaf->record[j].value[i];
				if ((max < candidate_value) && (old_max != candidate_value)) {
					max = candidate_value;	
				}
			}
			leaf->catalog.info[i].max = max;
		} else if (old_max < value[i]) {
			printf("catalog_delete: max < value err!!\n");
			exit(0);
		}
	}

	for (int i = 0; i < num_col - 1; i++) {
		min = leaf->catalog.info[i].min;
		max = leaf->catalog.info[i].max;

		printf("min: %ld, max: %ld\n", min, max);
	}
}

void 
catalog_insert(LeafPage* leaf, unumber_t num_col, unumber_t num_key, ukey64_t value[NMAXVAL]) {
	udata_t min, max;
	for (int i = 0; i < num_col - 1; i++) {
		if (num_key > 1) {
			min = leaf->catalog.info[i].min;
			max = leaf->catalog.info[i].max;
		} else {
			min = CATALOGINITMIN;
			max = CATALOGINITMAX;
		}
		if (min > value[i]) 
			leaf->catalog.info[i].min = value[i];
		
		if (max < value[i]) 
			leaf->catalog.info[i].max = value[i];
	}

	for (int i = 0; i < num_col - 1; i++) {
		min = leaf->catalog.info[i].min;
		max = leaf->catalog.info[i].max;

		printf("min: %ld, max: %ld\n", min, max);
	}

}

void
catalog_insert_split(LeafPage* leaf, unumber_t num_col, unumber_t num_key) {
	udata_t min, max, old_min, old_max;
	udata_t candidate_value;
	int index;
	for (int i = 0; i < num_col - 1; i++) {
		min = CATALOGINITMIN;
		max = CATALOGINITMAX;
	
		//get min
		for (int j = 0; j < num_key; j++) {
			candidate_value = leaf->record[j].value[i];
			if (min > candidate_value) {
				min = candidate_value;
			} 
		}
		leaf->catalog.info[i].min = min;

		//get max
		for (int j = 0;i < num_key; j++) {
			candidate_value = leaf->record[j].value[i];
			if (max < candidate_value) {
				max = candidate_value;	
			}
		}
		leaf->catalog.info[i].max = max;
	}

	for (int i = 0; i < num_col - 1; i++) {
		min = leaf->catalog.info[i].min;
		max = leaf->catalog.info[i].max;

		printf("min: %ld, max: %ld\n", min, max);
	}

}

uoffset_t 
find_leaf(utable_t tid, ukey64_t key, LeafPage* leaf_page) {
	int i = 0;
	InternalPage* inter_page;
	HeaderPage* hp = (HeaderPage*)malloc(sizeof(HeaderPage));
	inter_page = (InternalPage*)malloc(sizeof(InternalPage));

	read_buffer(tid, HEADEROFFSET, (Page*)hp);
	uoffset_t new_offset = hp->r_page_offset;	
	uoffset_t old_offset;
	if(new_offset == 0) {
		leaf_page = NULL;
		free(inter_page);
		return 0;
	}
	printf("newoff: %ld\n", hp->r_page_offset);

	//get page
	read_buffer(tid, new_offset, (Page*)inter_page);
	
	while (!inter_page->header_top.isLeaf) {
		i = 0;
		i = 1;
		while (i < inter_page->header_top.num_keys) {
			if (key >= inter_page->record[i].key) i++;
			else break;
		}
		old_offset = new_offset;
		new_offset = inter_page->record[i - 1].offset;

		//buffer_lock(new_offset);
		//buffer_release(old_offset);
		read_buffer(tid, new_offset, (Page*)inter_page);
	}
	memcpy(leaf_page, (LeafPage*)inter_page, PAGESIZE);
	free(inter_page);
	return new_offset;
}

/*
 *
 *
 *
 INSERT
 *
 *
 *
 */

void
insert_into_leaf(utable_t tid, uoffset_t offset, unumber_t num_col, ukey64_t key, udata_t value[], LeafPage* leaf) {
	int i, insertion_point;

	insertion_point = 0;
	while (insertion_point < leaf->header_top.num_keys && leaf->record[insertion_point].key < key) {
		insertion_point++;
	}

	for (i = leaf->header_top.num_keys; i > insertion_point; i--) {
		memcpy(&(leaf->record[i]), &(leaf->record[i - 1]), LRECORDSIZE);
	}
	leaf->record[insertion_point].key = key;
	memcpy(leaf->record[insertion_point].value, value, VALUESIZE);
	leaf->header_top.num_keys++;
	catalog_insert(leaf, num_col, leaf->header_top.num_keys, value);
	write_buffer(tid, offset, (Page*)leaf);
	return;
}




void
insert_into_new_root(utable_t tid, ukey64_t key_left, ukey64_t key_right, 
		uoffset_t left_offset, uoffset_t right_offset, NodePage* left, NodePage* right) {
	uoffset_t root_offset;
	HeaderPage* hp = (HeaderPage*)malloc(sizeof(HeaderPage));
	InternalPage* inter_page = (InternalPage*)malloc(sizeof(InternalPage));
	root_offset = init_root(tid, False);

	//poffset change
	left->header_top.poffset = root_offset;
	right->header_top.poffset = root_offset;

	
	//lock???
	read_buffer(tid, root_offset, (Page*)inter_page);
	read_buffer(tid, HEADEROFFSET, (Page*)hp);
	inter_page->record[0].key = key_left;
	inter_page->record[0].offset = left_offset;
	inter_page->record[1].key = key_right;
	inter_page->record[1].offset = right_offset;

	inter_page->header_top.num_keys += 2;
	inter_page->header_top.poffset = 0;

	hp->r_page_offset = root_offset;
	
	write_buffer(tid, left_offset, (Page*)left);
	write_buffer(tid, right_offset, (Page*)right);
	write_buffer(tid, root_offset, (Page*)inter_page);
	write_buffer(tid, HEADEROFFSET, (Page*)hp);
}

void
insert_into_node(utable_t tid, uoffset_t poffset, int left_index, ukey64_t key, uoffset_t right_offset, InternalPage* parent) {
	int i;
	//d_print_mpage(tid, (Page*)parent, 2);
	//printf("num key: %d\n", parent->header_top.num_keys);
	for (i = parent->header_top.num_keys; i > left_index + 1; i--) {
		//printf("[%d]\n", i);
		//LRECORDSIZE -> IRECORDSIZE case nanum
		if ( parent->header_top.isLeaf)
			memcpy(&(parent->record[i]), &(parent->record[i - 1]), LRECORDSIZE);
		else 
			memcpy(&(parent->record[i]), &(parent->record[i - 1]), IRECORDSIZE);
			
		//d_print_mpage(tid, (Page*)parent, 2);
	}
	parent->header_top.num_keys++;
	//d_print_mpage(tid, (Page*)parent, 2);
	parent->record[left_index + 1].offset = right_offset;
	parent->record[left_index + 1].key = key;


	//d_print_mpage(tid, (Page*)parent, 2);
	write_buffer(tid, poffset, (Page*)parent);
	return;
}

void
insert_into_node_after_splitting(int left_index, utable_t tid, ukey64_t key, uoffset_t right_offset, 
		uoffset_t old_offset, InternalPage* old_node) {
	int i, j, split;
	ukey64_t k_prime_left, k_prime_right;
	InternalPage * new_node, * child;
	uoffset_t new_node_offset;
	Irecord * temp_records;



	child = (InternalPage*)malloc(sizeof(InternalPage));
	temp_records = (Irecord*)malloc(sizeof(Irecord) * inter_order);
	new_node = (InternalPage*)malloc(sizeof(InternalPage));
	for (i = 0, j = 0; i < old_node->header_top.num_keys; i++, j++) {
		if (j == left_index + 1) j++;
		temp_records[j].key = old_node->record[i].key;
		temp_records[j].offset = old_node->record[i].offset;
	}
	temp_records[left_index + 1].offset = right_offset;
	temp_records[left_index + 1].key = key;

	split = cut(inter_order);
	make_inter(new_node);
	old_node->header_top.num_keys = 0;


	k_prime_left = temp_records[0].key;
	for (i = 0; i <= split - 1; i++) {
		memcpy(&(old_node->record[i]), &(temp_records[i]), IRECORDSIZE);
		old_node->header_top.num_keys++;
	}
	//old_node->record[i].offset = temp_records[i].offset;
	k_prime_right = temp_records[split].key;
	for (i, j = 0; i < inter_order; i++, j++) {
		memcpy(&(new_node->record[j]), &(temp_records[i]), IRECORDSIZE);
		new_node->header_top.num_keys++;
	}
	
	new_node->header_top.poffset = old_node->header_top.poffset;

	
	new_node_offset = alloc_page(tid);	//여기서 해당 buffer lock 한 채로
	write_buffer(tid, old_offset, (Page*)old_node);
	write_buffer(tid, new_node_offset, (Page*)new_node);

	//not sure =<
	for (i = 0; i < new_node->header_top.num_keys; i++) {
		//read lock
		read_buffer(tid, new_node->record[i].offset, (Page*)child); 
		// change write lock
		child->header_top.poffset = new_node_offset;
		write_buffer(tid, new_node->record[i].offset, (Page*)child);
		//release
	}

	insert_into_parent(tid, k_prime_left, k_prime_right, old_offset, new_node_offset, (NodePage*)old_node, (NodePage*)new_node);
	free(new_node);
	free(temp_records);
	return;
}

void
insert_into_parent(utable_t tid, ukey64_t key_left, ukey64_t key_right,
		uoffset_t left_offset, uoffset_t right_offset, NodePage* left, NodePage* right) {
	int left_index;
	uoffset_t poffset;
	InternalPage* parent = (InternalPage*)malloc(sizeof(InternalPage));
	poffset = left->header_top.poffset;
	
	//parent lock은 이걸 호출하는 함수에서 처음 걸어준다. 	
	//parent lock ==> write
	if(poffset == 0) {
		insert_into_new_root(tid, key_left, key_right, left_offset, right_offset, left, right);
		return;
	}
	read_buffer(tid, poffset, (Page*)parent);

	left_index = get_left_index(left_offset, parent);
	
	if(parent->header_top.num_keys < inter_order - 1) {
		insert_into_node(tid, poffset, left_index, key_right, right_offset, parent);
		return;
	}
	insert_into_node_after_splitting(left_index, tid, key_right, right_offset, poffset, parent);
}

void
insert_into_leaf_after_splitting(uoffset_t offset, utable_t tid, unumber_t num_col, ukey64_t key, udata_t value[], LeafPage* leaf) {
	LeafPage* new_leaf = (LeafPage*)malloc(sizeof(LeafPage));
	Lrecord* temp_records = (Lrecord*)malloc(sizeof(Lrecord) * leaf_order);
	int insertion_index, split, i, j;
	ukey64_t new_key, old_key;
	uoffset_t new_offset;

	make_leaf(new_leaf);

	insertion_index = 0;
	while (insertion_index < leaf_order - 1 && leaf->record[insertion_index].key < key)
		insertion_index++;

	for (i = 0, j = 0; i < leaf->header_top.num_keys; i++, j++) {
		if (j == insertion_index) j++;
		memcpy(&(temp_records[j]), &(leaf->record[i]), LRECORDSIZE);
	}
	
	temp_records[insertion_index].key = key;
	memcpy(temp_records[insertion_index].value, value, VALUESIZE);

	leaf->header_top.num_keys = 0;

	split = cut(leaf_order - 1);

	//leaf에
	for (i = 0; i < split; i++) {
		memcpy(&(leaf->record[i]), &(temp_records[i]), LRECORDSIZE);
		leaf->header_top.num_keys++;
	}

	for (i = split, j = 0; i < leaf_order; i++, j++) {
		memcpy(&(new_leaf->record[j]), &(temp_records[i]), LRECORDSIZE);
		new_leaf->header_top.num_keys++;
	}

	free(temp_records);

	new_leaf->sibling = leaf->sibling;
	new_leaf->header_top.poffset = leaf->header_top.poffset;
	for (i = new_leaf->header_top.num_keys; i < leaf_order - 1; i++)
		memset(new_leaf->record[i].value, 0, VALUESIZE);

	//buffer lock
	catalog_insert_split(new_leaf, num_col, new_leaf->header_top.num_keys);
	new_offset = alloc_page(tid);
	write_buffer(tid, new_offset, (Page*)new_leaf);
	//unlock은 페어런트 확장 호출 끝나고

	leaf->sibling = new_offset;
	for (i = leaf->header_top.num_keys; i < leaf_order - 1; i++)
		memset(leaf->record[i].value, 0, VALUESIZE);
	catalog_insert_split(leaf, num_col, leaf->header_top.num_keys);
	write_buffer(tid, offset, (Page*)leaf);
	
	new_key = new_leaf->record[0].key;
	old_key = leaf->record[0].key;
	
	insert_into_parent(tid, old_key, new_key, offset, new_offset, (NodePage*)leaf, (NodePage*)new_leaf);
}

/*
 *
 *
 *
 DELETE
 *
 *
 *
 */
int
get_neighbor_index(utable_t tid, uoffset_t offset, NodePage* knode) {
	int i;
	uoffset_t poffset;
	InternalPage parent;

	read_buffer(tid, knode->header_top.poffset, (Page*)&parent);

	for (i = 0; i <= parent.header_top.num_keys; i++) {
		if (parent.record[i].offset == offset)
			return i - 1;
	}
}

void
remove_entry_from_node(utable_t tid, uoffset_t koffset, uoffset_t toffset, unumber_t num_col, ukey64_t key, NodePage* knode) {
	int i, num_keys;
	int record_size;
	udata_t value[NMAXVAL];

	i = 0;
	if(knode->header_top.isLeaf) {
		LeafPage *kleaf = (LeafPage*)knode;
		//!=
		while (kleaf->record[i].key != key)
			i++;
		memcpy(value, kleaf->record[i].value, sizeof(udata_t) * NMAXVAL);
		for (++i; i < kleaf->header_top.num_keys; i++) 
			memcpy(&(kleaf->record[i - 1]), &(kleaf->record[i]), LRECORDSIZE);
		for (i = (--kleaf->header_top.num_keys); i < leaf_order; i++)
			memset(&(kleaf->record[i]), 0, LRECORDSIZE);
		catalog_delete(kleaf, num_col, kleaf->header_top.num_keys, value);
		write_buffer(tid, koffset, (Page*)kleaf);
	}
	else {
		InternalPage* kinter = (InternalPage*)knode;

		//!=
		i = 1;
		while ((i != kinter->header_top.num_keys) && (kinter->record[i].key != key))
			i++;
		if (i == kinter->header_top.num_keys) 
			i = 0;
		for (++i; i < kinter->header_top.num_keys; i++) 
			kinter->record[i - 1].key = kinter->record[i].key;
		
		i = 0;
		while (kinter->record[i].offset != toffset)
			i++;
		for (++i; i < kinter->header_top.num_keys; i++)
			kinter->record[i - 1].offset = kinter->record[i].offset;
		
		for (i = (--kinter->header_top.num_keys); i < inter_order; i++)
			memset(&(kinter->record[i]), 0, IRECORDSIZE);
		write_buffer(tid, koffset, (Page*)kinter);
	}
}

void
adjust_root(utable_t tid, uoffset_t roffset, NodePage* root) {
	NodePage new_root;
	HeaderPage* hp = (HeaderPage*)malloc(sizeof(HeaderPage));
	NodePage temp;
	uoffset_t new_offset;
	uoffset_t toffset;

	//printf("adjust start(%d)\n", ++adjust_count);
	if (root->header_top.isLeaf == True && root->header_top.num_keys > 0) {
		//printf("adjust done(%d)\n", --adjust_count);
		return;
	}
	if (root->header_top.isLeaf == False && root->header_top.num_keys > 1) {
		toffset = ((InternalPage*)root)->record[0].offset;
		read_buffer(tid, toffset, (Page*)&temp);
		write_buffer(tid, toffset, (Page*)&temp);
		//printf("adjust done(%d)\n", --adjust_count);
		return;
	}
	if (!(root->header_top.isLeaf)) {
		new_offset = ((InternalPage*)root)->record[0].offset;
		//buffer r lock
		dealloc_buffer(tid, roffset);
		read_buffer(tid, new_offset, (Page*)&new_root);
		read_buffer(tid, HEADEROFFSET, (Page*)hp);
		//header root lock
		//buffer w lock
		hp->r_page_offset = new_offset;
		write_buffer(tid, HEADEROFFSET, (Page*)hp);
		new_root.header_top.poffset = 0;
		write_buffer(tid, new_offset, (Page*)&new_root);
	}
	else {
		//header root lock
		//buffer w lock
		read_buffer(tid, HEADEROFFSET, (Page*)hp);
		hp->r_page_offset = INITOFFSET;
		write_buffer(tid, HEADEROFFSET, (Page*)hp);
		dealloc_buffer(tid, roffset);
	}
	//printf("adjust done(%d)\n", --adjust_count);
}

void 
coalesce_nodes (utable_t tid, int neighbor_index, ukey64_t k_prime, uoffset_t koffset,
		uoffset_t noffset, NodePage* knode, NodePage* neighbor) {

	int i, j, neighbor_insertion_index, n_end;
	uoffset_t toffset;
	uoffset_t poffset = 0;
	NodePage* change;
	NodePage* temp = (NodePage*)malloc(sizeof(NodePage));
	NodePage* parent = (NodePage*)malloc(sizeof(NodePage));
	//printf("coalesce start(%d)\n", ++coal_count);
	if (neighbor_index == -1) {
		change = knode;
		knode = neighbor;
		neighbor = change;
	}

	neighbor_insertion_index = neighbor->header_top.num_keys;
	
	//internal
	if (!knode->header_top.isLeaf) {
		InternalPage* ineighbor = (InternalPage*)neighbor;
		InternalPage* iknode = (InternalPage*)knode;
	
		ineighbor->record[neighbor_insertion_index].key = k_prime;
		ineighbor->record[neighbor_insertion_index].offset = iknode->record[0].offset;
		ineighbor->header_top.num_keys++;
		n_end = iknode->header_top.num_keys;
		iknode->header_top.num_keys--;
		for (i = neighbor_insertion_index + 1, j = 1; j < n_end; i++, j++) {
			memcpy(&(ineighbor->record[i]), &(iknode->record[j]), IRECORDSIZE);
			ineighbor->header_top.num_keys++;
			iknode->header_top.num_keys--;
		}
		for (i = 0; i < ineighbor->header_top.num_keys; i++) {
			toffset = ineighbor->record[i].offset;
			read_buffer(tid, toffset, (Page*)temp);

			temp->header_top.poffset = noffset;
			write_buffer(tid, toffset, (Page*)temp);
		}
	}

	//leaf
	else {
		LeafPage* lneighbor = (LeafPage*)neighbor;
		LeafPage* lknode = (LeafPage*)knode;

		for (i = neighbor_insertion_index, j = 0; j < lknode->header_top.num_keys; i++, j++) {
			memcpy(&(lneighbor->record[i]), &(lknode->record[j]), LRECORDSIZE);
			lneighbor->header_top.num_keys++;
		}
		lneighbor->sibling = lknode->sibling;
	}


	poffset = knode->header_top.poffset;
	read_buffer(tid, poffset, (Page*)parent);
	//neighbor->header_top.poffset = parent->header_top.poffset;
	write_buffer(tid, noffset, (Page*)neighbor);
	dealloc_buffer(tid, koffset);
	

	delete_entry(tid, poffset, koffset, k_prime, parent);
	//printf("coalesce done(%d)\n", --coal_count);
}


void 
delete_entry(utable_t tid, uoffset_t koffset, uoffset_t toffset, ukey64_t key, NodePage* knode) {
	int min_keys;
	HeaderPage * hp = (HeaderPage*)malloc(sizeof(HeaderPage));
	NodePage * neighbor = (NodePage*)malloc(sizeof(NodePage));
	InternalPage * parent = (InternalPage*)malloc(sizeof(InternalPage));
	int neighbor_index;
	int k_prime_index, k_prime;
	int capacity;
	uoffset_t noffset;
	unumber_t num_col;
	//read lock

	num_col = get_col(tid);
	read_buffer(tid, HEADEROFFSET, (Page*)hp);
	uoffset_t roffset = hp->r_page_offset;
	//printf("delete entry start(%d)\n", ++entry_count);
	remove_entry_from_node(tid, koffset, toffset, num_col, key, knode);
	if (knode->header_top.poffset == 0) {
		adjust_root(tid, roffset, knode);
		//printf("delete entry done(%d)\n", --entry_count);
		return;
	}

	min_keys = knode->header_top.isLeaf ? cut(leaf_order) : cut(inter_order);

	if (knode->header_top.num_keys >= min_keys) {
		write_buffer(tid, koffset, (Page*)knode);
		//printf("delete entry done(%d)\n", --entry_count);
		return;
	}
	neighbor_index = get_neighbor_index(tid, koffset, knode);
	read_buffer(tid, knode->header_top.poffset, (Page*)parent);
	k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
	k_prime = parent->record[k_prime_index + 1].key;
	noffset = neighbor_index == -1 ? parent->record[1].offset : parent->record[neighbor_index].offset;
	read_buffer(tid, noffset, (Page*)neighbor);
	capacity = knode->header_top.isLeaf ? LRECORD - 1 : IRECORD;
	// change if( knode->header_top.num_keys == 0 ) --> no need neighbor
	if ((knode->header_top.num_keys == 0 && knode->header_top.isLeaf == 1)
			|| (knode->header_top.num_keys == 1 && knode->header_top.isLeaf == 0)) {
		coalesce_nodes(tid, neighbor_index, k_prime, koffset, noffset, knode, neighbor);
	} 
	free(neighbor);
	free(parent);
	//printf("delete entry done(%d)\n", --entry_count);
}

udata_t* 
find(utable_t tid, ukey64_t key) {
	LeafPage* lp = (LeafPage*)malloc(sizeof(LeafPage));
	udata_t* ret = (udata_t*)malloc(sizeof(udata_t)*15);

	int i;
	find_leaf(tid, key, lp);

	//empty db
	if(lp == NULL) {
		free(ret);
		ret = NULL;
		return ret;
	}
	for(i = 0; i < lp->header_top.num_keys; i++) 
		if (lp->record[i].key == key) break;
	if(i == lp->header_top.num_keys) {
		free(ret);
		ret = NULL;
		return ret;
	}
	else {
		memcpy(ret, lp->record[i].value, VALUESIZE);
		return ret;
	}
}

void
insert(utable_t tid, ukey64_t key, udata_t value[]) {
	uoffset_t node_offset;
	unumber_t num_col;
	//ustring_t str = (char*)malloc(sizeof(char) * VALUESIZE);
	udata_t* temp;
	LeafPage* leaf_page = (LeafPage*)malloc(sizeof(LeafPage));
	HeaderPage* hp = (HeaderPage*)malloc(sizeof(HeaderPage));

	read_buffer(tid, HEADEROFFSET, (Page*)hp);

	node_offset = hp->r_page_offset;
	temp = find(tid, key);
	
	num_col = get_col(tid);
	//reset check value as num_col
	
	for (int i = num_col - 1; i < 15; i++) {
		value[i] = VUNUSED;
	}

	printf("insert: value: ");
	for (int i = 0; i < 15; i++) {
		printf("%ld ", value[i]);
	}
	printf("\n");


	//already have same key
	if (temp != NULL) {
		return;
	}

	//root is empty 
	if (node_offset == 0) {
		node_offset = init_root(tid, True);
		
		read_buffer(tid, node_offset, (Page*)leaf_page);
		leaf_page->record[0].key = key;
		memcpy(leaf_page->record[0].value, value, VALUESIZE);
		leaf_page->header_top.num_keys++;
		catalog_insert(leaf_page, num_col, leaf_page->header_top.num_keys, value);
		write_buffer(tid, node_offset, (Page*)leaf_page);	
		return;
	}
	
	node_offset = find_leaf(tid, key, leaf_page);
	// just write on that page
	if ( leaf_page->header_top.num_keys < leaf_order - 1) {
		//buffer lock change ( read lock -> write lock ) { node_offset }
		insert_into_leaf(tid, node_offset, num_col, key, value, leaf_page);
		//buffer release 	
		return;
	}
	
	//buffer lock change ( read_lock -> write+up lock ) { node_offset }
	insert_into_leaf_after_splitting(node_offset, tid, num_col, key, value, leaf_page);
}

void
erase(utable_t tid, ukey64_t key) {
	//ustring_t str = (char*)malloc(sizeof(char) * VALUESIZE);
	udata_t* temp;
	uoffset_t koffset;
	LeafPage* key_leaf = (LeafPage*)malloc(sizeof(LeafPage));



	temp = find(tid, key);
	koffset = find_leaf(tid, key, key_leaf);

	if(temp != NULL && key_leaf != NULL)
		delete_entry(tid, koffset, 0, key, (NodePage*)key_leaf);

	free(temp);
}


LeafPage* 
get_left_leaf(utable_t tid) {
	uoffset_t off;
	NodePage* np = (NodePage*)malloc(sizeof(NodePage));
	InternalPage* ip;// = (InternalPage*)malloc(sizeof(InternalPage));
	LeafPage* lp;// = (LeafPage*)malloc(sizeof(LeafPage));
	HeaderPage* hp = (HeaderPage*)malloc(sizeof(HeaderPage));
	read_buffer(tid, 0, (Page*)hp);
	off = hp->r_page_offset;
	if (off == 0) {
		//has no tree
		return NULL;
	}

	while(off != 0) {
		read_buffer(tid, off, (Page*)np);
		if(np->header_top.isLeaf) {
			lp = (LeafPage*)np;
			return lp;
		}
		else {
			ip = (InternalPage*)np;
			off = ip->record[0].offset;
		}
	}

}

unumber_t 
get_all_record(utable_t tid, std::vector<ColInfo> col_infos, std::vector<JoinData> join_datas) {
	LeafPage* lp = get_left_leaf(tid);
	
	
}

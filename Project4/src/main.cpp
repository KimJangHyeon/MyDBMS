#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include "types.h"
#include "params.h"
#include "join_struct.h"
#include "queue.h"
#include "pages.h"
#include "lock.h"
#include "tables.h"
#include "disks.h"
#include "buffers.h"
#include "views.h"
#include "join.h"
//========================
#include "utils.h"

#define NTEST 10
/*
int storetest(utable_t);
int reversetest(utable_t);
int smalltest(utable_t);
int inserttest(utable_t);
int rsmalltest(utable_t);
int endtest(utable_t);

int (*testfunc[NTEST])(utable_t) = {
    smalltest,
    inserttest,
    storetest,
    rsmalltest,
    reversetest,
    endtest,
};

char* testname[NTEST] = {
    "smalltest",
    "inserttest",
    "storetest",
    "rsmalltest",
    "reversetest",
    "end test",
};

int endtest(utable_t tid) {
    return 0;
}

int rsmalltest(utable_t tid) {
    for (int i = 18; i >= 0; i--) {
        insert(tid, i, "aa");
    }
    d_print_dpage(tid, 0, DHEADER);
    d_print_tree(tid);
    for (int i = 18; i >= 0; i--) {
        delete(tid, i);
        d_print_tree(tid);
    }
    d_print_dpage(tid, 0, DHEADER);
    d_print_tree(tid);
    return 1;
}
int inserttest(utable_t tid) {
    for (int i = 0; i < 100000; i++) {
        insert(tid, i, "aa");
    }
    d_print_dpage(tid, 0, DHEADER);
    d_print_tree(tid);
    for (int i = 0; i < 100000; i++) {
        delete(tid, i);
    	d_print_tree(tid);
    }
    d_print_dpage(tid, 0, DHEADER);
    d_print_tree(tid);
    return 1;
}


int smalltest(utable_t tid) {
    d_free_page_ditector(tid);
    insert(tid, 0, "a");
    d_print_tree(tid);
    insert(tid, 2, "a");
    d_print_tree(tid);
    insert(tid, 4, "a");
    d_print_tree(tid);
    insert(tid, 6, "a");
    d_print_tree(tid);
    insert(tid, 3, "a");
    d_print_tree(tid);
    insert(tid, 7, "a");
    d_print_tree(tid);
    //d_print_dpage(tid, 0, D_Header);
    delete(tid, 0);
    delete(tid, 2);
    delete(tid, 3);
    //d_print_dpage(tid, 0, D_Header);
    d_print_tree(tid);
    insert(tid, 0, "a");
    insert(tid, 2, "a");
    //d_print_dpage(tid, 0, D_Header);
    d_print_tree(tid);
    return 1;
}


int storetest(utable_t tid) {
    for(int i = 0; i < LRECORD * 10; i++) {
        insert(tid, i, "a");
    }
    d_print_dpage(tid, 0, DHEADER);
    d_print_tree(tid);
    for(int i = 0; i < LRECORD * 10; i++) {
        delete(tid, i); 
    }   
    d_print_dpage(tid, 0, DHEADER);
    d_print_tree(tid);
    return 1;
}


int reversetest(utable_t tid) {
    for(int i = LRECORD * 10 - 1; i >= 0; i--) {
        insert(tid, i, "a");
    }  
    d_print_dpage(tid, 0, DHEADER);
    d_print_tree(tid);
    for(int i = LRECORD * 10 - 1; i >= 0; i--) {
        delete(tid, i); 
        d_print_tree(tid);
    }   
    d_print_dpage(tid, 0, DHEADER);
    d_print_tree(tid);
    return 1;
}


void 
print_test(int i, bool isStart) {
    printf("*\n*\n");
    printf("%s ", testname[i]);
    if(isStart)
        printf("start");
    else 
        printf("done");
    printf("\n*\n*\n");
}*/
void test(utable_t tid) {
/*
	int ret;
    for (int i = 0; i < NTEST; i++) {
        print_test(i, True);
        ret = testfunc[i](tid);
        print_test(i, False);
        if (!ret) 
            break;
    }   */
}

char client() {
	char choice;
	printf("****************\nopen: o\nclose: c\ninsert: i\ndelete: d\nfind: f\ntest: t\nprint tree: p\nquit: q\n*****************\n");
	printf("> ");
	scanf("%c", &choice);
	return choice;
}

void client_loop() {
    char table_path[64];
	utable_t tid;
	int res;
	int j = 0;
	unumber_t num_col;
	ukey64_t key;
	udata_t* value = (udata_t*)malloc(sizeof(udata_t)* 15);
	udata_t* temp;
	while(1) {
		switch(client()) {
			case 'o':
				scanf("%s %ld", table_path, &num_col);
				tid = open_table(table_path, num_col);
				if (tid == 0) {
					printf("the table is already opened!\n");
					break;
				}
				if (tid == -1) {
					printf("table pool is full!!\n");
					break;
				}
				printf("tid: %ld\n", tid);
				break;
			case 'c':
				scanf("%ld", &tid);
				res = close_table(tid);
				if (res == -1) {
					printf("tid range err!\n");
					break;
				} else if (res == 1) {
					printf("no such tid!\n");
					break;
				} else {
					printf("success!\n");
					break;
				}
			case 'i':
				scanf("%ld %ld", &tid, &key);
				j = 0;
				//memset(value, 0, sizeof(udata_t) * 15);
				for (int i = 0; i < 15; i++) {
					value[i] = 0;
				}
				
				// one missing input	
				do {
					if (j < 15) 
						scanf("%ld", &(value[j++]));
				} while(getchar() != '\n'); 
				insert(tid, key, value);
				break;
			case 'd':
				scanf("%ld %ld", &tid, &key);
				erase(tid, key); 
				break;
			case 'f':
				scanf("%ld %ld", &tid, &key);
				temp = find(tid, key);
				if (temp == NULL) {
					printf("no such a key!!\n");
				}
				else {
					printf("find: ");
					for (int i = 0; i < 15; i++) {
						if (temp[i] != VUNUSED)
							printf("%ld ", temp[i]);
						else
							break;
					}
				}
				printf("\n");
				break;
			case 't':
				scanf("%ld", &tid);
				test(tid);
				break;
			case 'l':
				debug_lru();
				break;
			case 'p':
				scanf("%ld", &tid);
				d_print_tree(tid);
				break;
			case 'q':
				shutdown_db();
				exit(0);
				break;
		}
		while(getchar() != '\n');
	}
}
///HEERE
//FIX CLOSE TABLE return 0 -1 1??
// if -1 return no such tid
// if 0 success
// if 1 fail



int
main (int argc, char ** argv) {
	
    char* table_path;
	int num_buf;
	utable_t tid1, tid2, tid3, tid4, tid5, tid6;
	udata_t* value;
    if(argc > 1 && argc < 3) {
		num_buf = atoi(argv[1]);
    } else {
        panic((char*)"panic for input(buffer.c)");
    }
//	init_db(num_buf);
//	client_loop();
	init_db(num_buf);

	tid1 = open_table("a", 3);
	tid2 = open_table("b", 3);
	tid3 = open_table("c", 3);
	tid4 = open_table("d", 3);
	value = (udata_t*)malloc(sizeof(udata_t) * 15);
	int j = 0;
	for (int i = 0; i < 15; i++) {
		value[i] = 0;
	}
				
	// one missing input	
	//do {
	//	if (j < 15) 
	//		scanf("%ld", &(value[j++]));
	//} while(getchar() != '\n'); 

	for (int i = 1; i < 11; i++) {
		value[0] = i*3 +1;
		value[1] = i*3 +2;
		insert(tid1, i*3, value);
	}
	d_print_tree(tid1);

	for (int i = 1; i < 11; i++) {
		value[0] = i*3;
		value[1] = i*3+1;
		insert(tid2, i*3+2, value);
	}
	
	for (int i = 1; i < 5; i++) {
		value[0] = i*3 + 1;
		value[1] = i*3 + 2;
		insert(tid3, i*3, value);
	}

	for (int i = 10; i < 16; i++) {
		value[0] = i*3 + 1;
		value[1] = i*3 + 2;
		insert(tid4, i*3, value);
	}
	d_print_tree(tid2);
	JoinSet join_set;
	JoinTree join_tree;

	join_set.parser("1.3=2.2&3.2=1.2&4.1=3.1&4.2=1.2&3.1=2.3");
	join_set.scanner();
	join_set.table_info_print();
	join_set.join_info_print();

	std::cout << "****************************" << std::endl;

	join_set.order_by_join();
	join_set.join_info_print();
//	join_set.table_info_print();
	std::cout << "************ JOIN TREE ****************" << std::endl;
	join_tree.make_tree(join_set.join_info, join_set.table_info);
	join_tree.join_tree_print();
	for (int i = 0; i < join_tree.join_point_size(); i++) 
		join_tree.join(join_tree.join_address(i));
	
	join_tree.join_tree_print();
	close_table(tid1);
	close_table(tid2);

	
	//test(tid);
}

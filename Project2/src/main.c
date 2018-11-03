#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "params.h"
#include "queue.h"
#include "pages.h"
#include "lock.h"
#include "tables.h"
#include "disks.h"
#include "buffers.h"
#include "views.h"
//========================
#include "utils.h"

#define NTEST 10

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
    for (int i = 0; i < 18; i++) {
        insert(tid, i, "aa");
    }
    d_print_dpage(tid, 0, DHEADER);
    d_print_tree(tid);
    for (int i = 0; i < 18; i++) {
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
}
void test(utable_t tid) {
    int ret;
    for (int i = 0; i < NTEST; i++) {
        print_test(i, True);
        ret = testfunc[i](tid);
        print_test(i, False);
        if (!ret) 
            break;
    }   
}

char client() {
	char choice;
	printf("****************\nopen: o\nclose: c\ninsert: i\ndelete: d\nfind: f\ntest: t\nprint tree: p\n*****************\n");
	printf("> ");
	scanf("%c", &choice);
	return choice;
}

void client_loop() {
    char table_path[64];
	utable_t tid;
	ukey64_t key;
	ustring_t value = (ustring_t)malloc(sizeof(char)* 120);

	while(1) {
		switch(client()) {
			case 'o':
				scanf("%s", table_path);
				tid = open_table(table_path);
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
				close_table(tid);
				break;
			case 'i':
				scanf("%ld %ld %s", &tid, &key, value);
				insert(tid, key, value);
				break;
			case 'd':
				scanf("%ld %ld", &tid, &key);
				delete(tid, key); 
				break;
			case 'f':
				scanf("%ld %ld", &tid, &key);
				find(tid, key, &value);
				printf("find: %s\n", value);
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
		}
		while(getchar() != '\n');
	}
}


int
main (int argc, char ** argv) {
	
    char* table_path;
	int num_buf;
    if(argc > 1 && argc < 3) {
		num_buf = atoi(argv[1]);
    } else {
        panic("panic for input(buffer.c)");
    }
	/*
	init_tablepool();
	utable_t tid = open_table(table_path);
	test(tid);
	*/
	init_db(num_buf);
	client_loop();
	//test(tid);
}

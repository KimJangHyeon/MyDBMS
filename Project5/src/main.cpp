#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <pthread.h>

using namespace std;

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
#include "cc.h"
#include "utils.h"

udata_t cc_d1 = 1;
udata_t cc_d2 = 1;

pthread_spinlock_t print_lock;

void*
cctest(void* arg){
	utable_t tid = (utable_t)arg;
	udata_t value[15];
	int txn_id = 1;
	int result;
	udata_t* temp;
	
	value[0] = cc_d1;
	value[1] = cc_d2;

	for (int i = 0; i < 10000; i++) {
		if (i % 100 == 0) {
			__sync_fetch_and_add(&cc_d1, 1);
			__sync_fetch_and_add(&cc_d2, 1);
			value[0] = cc_d1;
			value[1] = cc_d2;
			update(tid, 1, value, txn_id, &result);
		} else {
			pthread_spin_lock(&print_lock);
			temp = find(tid, 1, txn_id, &result);	
			
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
			pthread_spin_unlock(&print_lock);
		}
	}

}

void 
test(utable_t tid, int v_size) {
	pthread_t thread[5];
	udata_t value[15];
	pthread_spin_init(&print_lock, NULL);
	for (int i = 0; i < v_size - 1; i++) {
		value[i] = 1;
	}
	for (int i = v_size - 1; i < 15; i++) {
		value[i] = 0;
	}
	for (ukey64_t i = 0; i < 10000; i++) {
		insert(tid, i, value); 
	}
	fprintf(stderr, "TEST START!!\n");	
	for (int i = 0; i < 5; i++) {
		pthread_create(&(thread[i]), NULL, cctest, (void*)tid);
	}

	for (int i = 0; i < 5; i++) {
		pthread_join(thread[i], NULL);
	}

}

void 
tokenize(const std::string& s, const char delim, udata_t value[15]) {
	std::string::size_type beg = 0;
	std::string t;
	int j = 0;
	for (auto end = 0; (end = s.find(delim, end)) != std::string::npos;++end) {
		beg = end + 1;
		t = s.substr(beg, end-beg);
		if (t.size() != 0)
			value[j++] = std::stoul(s.substr(beg, end-beg), nullptr, 0);	
	}
	//value[j] = std::stoul(s.substr(beg), nullptr, 0);
}

char client() {
	char choice;
	//printf("****************\nopen: o\nclose: c\ninsert: i\ndelete: d\nfind: f\ntest: t\nprint tree: p\nquit: q\n*****************\n");
	printf("> ");
	scanf("%c", &choice);
	return choice;
}

void client_loop() {
    char table_path[64];
	utable_t tid;
	int txn_id;
	int res;
	int j = 0;
	unumber_t num_col;
	ukey64_t key;
	udata_t* value = (udata_t*)malloc(sizeof(udata_t)* 15);
	udata_t* temp;
	std::string input;
	std::vector<std::string> token;
	std::string query;
	JoinSet join_set = JoinSet();
	JoinTree join_tree = JoinTree();
		

	
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
				printf("%ld\n", tid);
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
				
				std::cin >> tid;
				std::cin >> key;
				std::getline(std::cin, input);
				
				j = 0;
				for (int i = 0; i < 15; i++) {
					value[i] = 0;
				}
				
				tokenize(input, ' ', value);
				insert(tid, key, value);
				break;

			case 'u':
				std::cin >> tid;
				std::cin >> txn_id;
				std::cin >> key;
				std::getline(std::cin, input);
				
				j = 0;
				for (int i = 0; i < 15; i++) {
					value[i] = 0;
				}
				
				tokenize(input, ' ', value);
				update(tid, key, value, txn_id, &res);
				break;

			case 'd':
				scanf("%ld %ld", &tid, &key);
				erase(tid, key); 
				break;
			case 'f':
				scanf("%ld %d %ld", &tid, &txn_id, &key);
				temp = find(tid, key, txn_id, &res);
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
			case 'j':
				std::cin >> query;
				join_set.parser(query);
				join_set.scanner();	

				//join_set.join_order_print();
				join_tree.make_tree(join_set.join_infos, join_set.join_datas);
				std::cout << join_tree.join_all() << std::endl;
				break;
		
			case 't':
				scanf("%ld %d", &tid, &j);
				test(tid, j);
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
	init_db(num_buf);
	client_loop();
	/*init_db(num_buf);
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

	for (int i = 1; i < 1000; i++) {
		value[0] = i*3 +1;
		value[1] = i*3 +2;
		insert(tid1, i*3, value);
	}

	for (int i = 332; i < 1300; i++) {
		value[0] = i*3;
		value[1] = i*3+1;
		insert(tid2, i*3+2, value);
	}
	
	for (int i = 703; i < 1500; i++) {
		value[0] = i*3 + 1;
		value[1] = i*3 + 2;
		insert(tid3, i*3, value);
	}

	for (int i = 580; i < 1600; i++) {
		value[0] = i*3 + 1;
		value[1] = i*3 + 2;
		insert(tid4, i*3, value);
	}
	

	JoinSet join_set;
	JoinTree join_tree;
				//	 2    2 . 2   2 . 2   2 . 0   0
	join_set.parser("1.3=2.1&4.3=3.3&1.2=3.2&3.1=2.2");//.2&2.1=4.3&3.1=4.1");//&3.2=1.2&4.1=3.1&4.2=1.2&3.1=2.3");
	join_set.scanner();	

	//join_set.join_order_print();
	join_tree.make_tree(join_set.join_infos, join_set.join_datas);
	std::cout << join_tree.join_all() << std::endl;
	
	close_table(tid1);
	close_table(tid2);
	close_table(tid3);
	close_table(tid4);

	
	//test(tid);
*/
}

#include <stdio.h>
#include <stdlib.h>
#include "param.h"
#include "table.h"
#include "disk.h"
int main() {
	init_tablepool();
	char* path = "ddb";
	int tid = open_table(path);

	print_tp();
	open_disk(tid);	
	print_tp();
	close_disk(tid);
	print_tp();
	open_disk(tid);	
	print_tp();

}

#include <stdio.h>
#include "param.h"
#include "table.h"

int main() {
	init_tablepool();
	
	printf("tid: %d\n", open_table("abd"));
	printf("tid: %d\n", open_table("abc"));
	printf("tid: %d\n", open_table("abf"));
	printf("tid: %d\n", open_table("abg"));
	printf("tid: %d\n", open_table("abh"));
	printf("tid: %d\n", open_table("abt"));
	printf("tid: %d\n", open_table("ab3"));
	printf("tid: %d\n", open_table("aqwe"));
	printf("tid: %d\n", open_table("ase"));
	printf("tid: %d\n", open_table("aewwee"));
	printf("tid: %d\n", open_table("aewvwee"));
	printf("tid: %d\n", open_table("vaewvwee"));
	printf("close: %d\n", close_table(9));
	printf("close: %d\n", close_table(1));

	print_tp();
}

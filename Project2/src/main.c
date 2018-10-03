#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "params.h"
#include "pages.h"
#include "tables.h"
#include "disks.h"
//========================
#include "utils.h"
int main() {
	init_tablepool();
	char* path = "ddb";
	int tid = open_table(path);

	d_free_page_ditector(tid);
}

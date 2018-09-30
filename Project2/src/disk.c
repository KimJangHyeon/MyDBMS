#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "param.h"
#include "table.h" 
int
open_disk(int tid) {
        char* db_path;
        int fd;
        db_path = get_path(tid);
	if((fd = open(db_path, O_CREAT | O_RDWR, 0777)) == -1) {
		printf("path: %s\n", db_path);
        	fprintf(stderr, "Write error: %s\n", strerror(errno));
        	//panic("(disk.c)open file error");
		return -1;
	}
	put_fd(tid, fd);
	return fd; 
}

int
close_disk(int tid) {
	int fd;
        fd = get_fd(tid);
        close(fd);
        put_fd(tid, FDCLOSE);

}


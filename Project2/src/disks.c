#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "params.h"
#include "types.h"
#include "pages.h"
#include "tables.h" 
#include "utils.h"
int
open_disk(int tid) {
        char* db_path;
        int fd;
        db_path = get_path(tid);
	if((fd = open(db_path, O_CREAT | O_RDWR, 0777)) == -1) {
		printf("path: %s\n", db_path);
        	fprintf(stderr, "Write error: %s\n", strerror(errno));
        	panic("(disk.c)open file error");
		return -1;
	}

	put_fd(tid, fd);
	
	if( lseek(fd, 0, SEEK_END) == 0 ) {
		close_disk(tid);
		init_table(tid);
		fd = open_disk(tid);
	}
	return fd; 
}

int
close_disk(int tid) {
	int fd;
        fd = get_fd(tid);
        close(fd);
        put_fd(tid, FDCLOSE);

}

//use not in buffer 
//only for free page ??
void
flush_page(utable_t tid, uoffset_t offset, Page* page) {
        int fd;
        int size;

        open_disk(tid);
        fd = get_fd(tid);
        if((size = pwrite(fd, page, PAGESIZE, offset)) == -1) {
                printf("fd: %d, page sz: %lu, offset: %lu\n", fd, sizeof(Page), offset);

                fprintf(stderr, "OPEN ERROR: %s\n", strerror(errno));
                panic("(disk.c) flush page pwrite panic!!");
        }
        close_disk(tid);
        if(size != PAGESIZE)
                panic("(flush_page) writen page size is less than 4096");
}

void
load_page(utable_t tid, uoffset_t offset, Page* page) {
        int size;
        int fd;
        open_disk(tid);
        fd = get_fd(tid);
        if((size = pread(fd, page, PAGESIZE, offset)) == -1) {
                printf("READ ERR: %s\n", strerror(errno));
                panic("(load_page) load page pread panic!!\n");
        }
        close_disk(tid);
        if(size != PAGESIZE) {
                printf("d_size: %d\n", size);
                panic("(load_page) page size do not match panic!!\n");
        }
}

uoffset_t 
alloc_page(utable_t tid) {

}

void
dealloc_page(utable_t tid, uoffset_t offset) {

}

//expand_after alloc 
void 
expand_page(utable_t tid, int size ) {
		
}

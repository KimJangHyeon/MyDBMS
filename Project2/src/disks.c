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
#include "disks.h"
#include "inits.h"
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
extend_page(utable_t tid, int size ) {
    //이 함수 호출 이전에 expand lock 하고 ㄱㄱ 
    FreePage first_free_page;
    HeaderPage* m_header = (HeaderPage*)malloc(sizeof(HeaderPage));
    uoffset_t num_cur_pages;
    
    //read_buffer(tid, C_HeaderOffset, (Page*)m_header);
	load_page(tid, HEADEROFFSET, (Page*)m_header);

    num_cur_pages = m_header->number_of_pages;
    
    FreePage* free_page = (FreePage*)malloc(sizeof(FreePage));
    uoffset_t free_page_offset = size * PAGESIZE;
    uoffset_t expand_limit;
    if(size == 1)
        expand_limit = 8 * PAGESIZE;
    else
        expand_limit = (size * 2) * PAGESIZE;

    memset(free_page, '\0', PAGESIZE);
    for(; free_page_offset < expand_limit; free_page_offset += PAGESIZE) {
        free_page->next_free_page = free_page_offset - PAGESIZE;
        flush_page(tid, free_page_offset, (Page*)free_page);
    }
    free(free_page);
    
    load_page(tid, num_cur_pages * PAGESIZE, (Page*)&first_free_page);
    first_free_page.next_free_page = m_header->f_page_offset;
    flush_page(tid, num_cur_pages * PAGESIZE, (Page*)&first_free_page);

    m_header->f_page_offset = (size + num_cur_pages - 1) * PAGESIZE;
    m_header->number_of_pages += size;
    m_header->number_of_free_pages += size;
    flush_page(tid, HEADEROFFSET, (Page*)m_header);
}




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
#include "lock.h"
#include "queue.h"
#include "pages.h"
#include "tables.h" 
#include "utils.h"
#include "disks.h"
#include "buffers.h"
#include "inits.h"
int
open_disk(utable_t tid) {
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
	return fd; 
}


int
close_disk(utable_t tid) {
	int fd;
        fd = get_fd(tid);
        close(fd);
        put_fd(tid, FDCLOSE);

}

unumber_t
disk_size(utable_t tid) {
	int fd = get_fd(tid);
	unumber_t ret = lseek(fd, 0, SEEK_END);
	return ret;
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
    uoffset_t alloc_offset;
    FreePage free_page;
    HeaderPage* m_header = (HeaderPage*)malloc(sizeof(HeaderPage));
    unumber_t cur_free_page_num;
    unumber_t cur_page_num;
   
	read_buffer(tid, HEADEROFFSET, (Page*)m_header);

	//if free page 0
	while (m_header->number_of_free_pages == 0) {
		printf("WARNING: free pages are 0\n");
		read_buffer(tid, HEADEROFFSET, (Page*)m_header);
	}
    
	alloc_offset = m_header->f_page_offset;
    read_buffer(tid, alloc_offset, (Page*)&free_page);
    m_header->f_page_offset = free_page.next_free_page;
    m_header->number_of_free_pages -= 1;
    cur_free_page_num = m_header->number_of_free_pages;
    cur_page_num = m_header->number_of_pages;


    write_buffer(tid, HEADEROFFSET, (Page*)m_header);
    
	if(EXTENDPERCENT >= ((double)cur_free_page_num / (double)cur_page_num)) {
        //extend thread call --> 현재 있는 thread를 작동 시킨다.(새로 생성하면 extend끼리 겹>칠 수 있음-done-)
        //extend create 가능하지만 alloc thread가 죽으면 어쩔거?
        //main thread가 지할거 다하고 기다린다?? (join)
        extend_page(tid, cur_page_num);
    }

    return alloc_offset;
}

void
dealloc_page(utable_t tid, uoffset_t offset) {
    HeaderPage* m_header = (HeaderPage*)malloc(sizeof(HeaderPage));
    FreePage free_page;
    
    memset(&free_page, offset, PAGESIZE);
    
    read_buffer(tid, HEADEROFFSET, (Page*)m_header);
    
	free_page.next_free_page = m_header->f_page_offset;
    m_header->f_page_offset = offset;
    m_header->number_of_free_pages++;
    flush_page(tid, offset, (Page*)&free_page);
    write_buffer(tid, HEADEROFFSET, (Page*)m_header);
}

void 
extend_page(utable_t tid, int size ) {
    HeaderPage* m_header = (HeaderPage*)malloc(sizeof(HeaderPage));
    FreePage* free_page = (FreePage*)malloc(sizeof(FreePage));
    FreePage first_free_page;
    uoffset_t num_cur_pages;
    uoffset_t expand_limit;
    
    read_buffer(tid, HEADEROFFSET, (Page*)m_header);
    num_cur_pages = m_header->number_of_pages;
    uoffset_t free_page_offset = num_cur_pages * PAGESIZE;	
	expand_limit = (size + num_cur_pages) * PAGESIZE;
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
    write_buffer(tid, HEADEROFFSET, (Page*)m_header);
}




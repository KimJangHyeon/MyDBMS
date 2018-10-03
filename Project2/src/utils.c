#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "types.h"
#include "params.h"
#include "pages.h"
#include "disks.h"
#include "utils.h"
void 
panic(char* error) {
        printf("panic: %s\n", error);
        exit(0);
}


void
d_print_dheader(utable_t tid) {
        HeaderPage hp;
        load_page(tid, 0, (Page*)&hp);

        printf("*********** header ***********\n");
        printf("free page offset:     %lu\n", hp.f_page_offset);
        printf("root page offset:     %lu\n", hp.r_page_offset);
        printf("number of pages:      %lu\n", hp.number_of_pages);
        printf("number of free pages: %lu\n", hp.number_of_free_pages);
        printf("******************************\n\n");

}

void
d_print_mheader(utable_t tid) {
        HeaderPage * hp = (HeaderPage*)malloc(sizeof(HeaderPage));
        //read_buffer(tid, HEADEROFFSET, (Page*)hp);
        load_page(tid, HEADEROFFSET, (Page*)hp);
	printf("******************M_Header******************\n");
        printf("free page: %lu\n", hp->f_page_offset);
        printf("root page: %lu\n", hp->r_page_offset);
        printf("num page : %lu\n", hp->number_of_pages);
        printf("num fpage: %lu\n", hp->number_of_free_pages);
        printf("********************************************\n");
}

void 
d_print_mfree(FreePage fp) {
        printf("**************** free ******************\n");
        printf("next free: %lu \n\n",fp.next_free_page);
        printf("****************************************\n");
}
void 
d_print_dfree(utable_t tid, uoffset_t offset) {
        FreePage fp;

        load_page(tid, offset, (Page*)&fp);
        printf("**************** free ******************\n");
        printf("next free: %lu \n\n",fp.next_free_page);
        printf("****************************************\n");
}

void
d_print_mnode(NodePage np) {
        LeafPage lp;
        InternalPage ip;
        if(np.header_top.isLeaf) {
                printf("**************** leaf *****************\n");
                memcpy(&lp, &np, PAGESIZE);
                printf("poffset: %lu \n", lp.header_top.poffset);
                printf("is leaf: %d\n", lp.header_top.isLeaf);
                printf("num key: %d\n", lp.header_top.num_keys);
                printf("sibling: %ld\n", lp.sibling);
                for( int i = 0; i < lp.header_top.num_keys; i++) {
                        printf("[%d]\n", i);
                        printf("key: %ld\n", lp.record[i].key);
                        printf("value: %s\n", (lp.record[i].value));
                }
        }
        else {
                printf("**************** inter *****************\n");
                memcpy(&ip, &np, PAGESIZE);
                printf("poffset: %lu \n", ip.header_top.poffset);
                printf("is leaf: %d\n", ip.header_top.isLeaf);
                printf("num key: %d\n", ip.header_top.num_keys);
                for( int i = 0; i < ip.header_top.num_keys; i++) {
                        printf("[%d]\n", i);
                        printf("key: %ld\n", ip.record[i].key);
                        printf("value: %ld\n", ip.record[i].offset);

                }
        }
        printf("*************************************\n");
}

void
d_print_dnode(utable_t tid, uoffset_t offset) {
        LeafPage lp;
        InternalPage ip;
        NodePage np;

        load_page(tid, offset, (Page*)&np);
        if(np.header_top.isLeaf) {
                printf("**************** leaf *****************\n");
                memcpy(&lp, &np, PAGESIZE);
                printf("poffset: %lu \n", lp.header_top.poffset);
                printf("is leaf: %d\n", lp.header_top.isLeaf);
                printf("num key: %d\n", lp.header_top.num_keys);
                printf("sibling: %ld\n", lp.sibling);
                for( int i = 0; i < lp.header_top.num_keys; i++) {
                        printf("[%d]\n", i);
                        printf("key: %ld\n", lp.record[i].key);
                        printf("value: %s\n", (lp.record[i].value));
                }
        }
        else {
                printf("**************** inter *****************\n");
                memcpy(&ip, &np, PAGESIZE);
                printf("poffset: %lu \n", ip.header_top.poffset);
                printf("is leaf: %d\n", ip.header_top.isLeaf);
                printf("num key: %d\n", ip.header_top.num_keys);
                for( int i = 0; i < ip.header_top.num_keys; i++) {
                        printf("[%d]\n", i);
                        printf("key: %ld\n", ip.record[i].key);
                        printf("value: %ld\n", ip.record[i].offset);

                }
        }
        printf("*************************************\n");
}


void 
d_print_dpage(utable_t tid, uoffset_t offset, int type) {
        if (type == DHEADER)
                d_print_dheader(tid);
        else if (type == DFREE)
                d_print_dfree(tid, offset);
        else
                d_print_dnode(tid, offset);
}

void 
d_print_mpage(utable_t tid, Page* page, int type) {
        if (type == DHEADER)
                d_print_mheader(tid);
        else if (type == DFREE)
                d_print_mfree(*((FreePage*)page));
        else
                d_print_mnode(*((NodePage*)page));
}

void 
d_free_page_ditector(utable_t tid) {
        HeaderPage hp;
        FreePage fp;
        uoffset_t old_free;
        uoffset_t free_offset;
        load_page(tid, 0, (Page*)&hp);
        free_offset = hp.f_page_offset;
        d_print_dpage(tid, 0, DHEADER);
        d_print_mpage(tid, (Page*)&hp, DHEADER);
        do {
                load_page(tid, free_offset, (Page*)&fp);
                old_free = free_offset;
                free_offset = fp.next_free_page;
                printf("******** free **************\n");
                printf("free_offset: %lu(%lu)\n", old_free, old_free/PAGESIZE);
                printf("next free offset: %lu(%lu)\n", fp.next_free_page, fp.next_free_page/PAGESIZE);
                printf("nfo/4096: %lu\n", fp.next_free_page/4096);
                printf("****************************\n");
        } while(fp.next_free_page != 0); 
}



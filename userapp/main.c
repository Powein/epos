/*
 * vim: filetype=c:fenc=utf-8:ts=4:et:sw=4:sts=4
 */
#include <inttypes.h>
#include <stddef.h>
#include <math.h>
#include <stdio.h>
#include <sys/mman.h>
#include <syscall.h>
#include <netinet/in.h>
#include <stdlib.h>
#include "graphics.h"

extern void *g_heap;
/**
 * GCC insists on __main
 *    http://gcc.gnu.org/onlinedocs/gccint/Collect2.html
 */
void __main()
{
    size_t heap_size = 32*1024*1024;
    void  *heap_base = mmap(NULL, heap_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
	// g_heap = tlsf_create_with_pool(heap_base, heap_size);
    g_heap = init_memory_pool(heap_size, heap_base);
}

void main(void *pv){

    printf("task #%d: I'm the first user task(pv=0x%08x)!\r\n",task_getid(), pv);

    
    extern void test_allocator();
    test_allocator();
    while(1)
        ;
    task_exit(0);
}


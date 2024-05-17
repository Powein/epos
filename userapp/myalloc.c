/**
 * vim: filetype=c:fenc=utf-8:ts=4:et:sw=4:sts=4
 */
#include <sys/types.h>
#include <string.h>
#include <stdint.h>

struct chunk {
    char signature[4];  /* "OSEX" */
    struct chunk *next; /* ptr. to next chunk */
    int state;          /* 0 - free, 1 - used */
#define FREE   0
#define USED   1

    int size;           /* size of this chunk */
};

static struct chunk *chunk_head;

void init_memory_pool(size_t heap_size, uint8_t *heap_base)
{
    chunk_head = (struct chunk *)heap_base;
    strncpy(chunk_head->signature, "OSEX", 4);
    chunk_head->next = NULL;
    chunk_head->state = FREE;
    chunk_head->size  = heap_size;
}

// void *malloc(size_t size)
// {
//     if(size == 0)
//         return NULL;
//     struct chunk* p = chunk_head;
//     while(p != NULL) {
//         if(p->state == FREE && p-> size >= size) {
//             p->state = USED;
//             struct chunk* q = (struct chunk*)((char*)p + size);
//             q->state = FREE;
//             q->size = p->size - size;
//             strncpy(q->signature, "OSEX", 4);
//             struct chunk* temp = p->next;
//             p->next = q;
//             q->next = temp;
//             return (void*)((char*)p + sizeof(struct chunk));
//         }
//         p = p->next;
//     }
// }


void *malloc(size_t size){
    if(size==0)
        return NULL;
    //find a free block with enough size
    struct chunk *free_block = NULL;
    free_block = chunk_head;
    while(free_block) {
        if (free_block->state == FREE && free_block->size >= size){
            break;
        }
        free_block = free_block->next;
    }
    //no such block found
    if(free_block==NULL)
        return NULL;

    void *ptr;
    if(strncmp(free_block->signature,"OSEX",4) == 0)//legal block to use
        ptr=(char *)free_block+sizeof(struct chunk);//get the usable address
    else
        return NULL;
    int freesize=free_block->size - size - sizeof(struct chunk);//freesize for fragment
    if(freesize <= 0) {
        free_block->state = USED;//freesize is too small to use
        return ptr;
    }
    else {//add a new block for the remaining space
        struct chunk* new = (struct chunk*)((char*)ptr+size);
        // *new=add_block(freesize);//add a new block for the remaining space
        //initialize the new block
        strncpy(new->signature, "OSEX", 4);
        new->state=FREE;
        new->size=freesize;
        free_block->size=size;
        free_block->state=USED;
        new->next=free_block->next;
        free_block->next=new;
        return ptr;
    }
}


void free(void *ptr)
{
    ptr = ptr - sizeof(struct chunk);
    struct chunk *p = (struct chunk*)ptr;
    p->state = FREE;
    struct chunk *merger = NULL;
    merger = chunk_head;
    while(merger!= NULL && merger->next!= NULL) {
        if (merger->next->state == FREE && merger->state == FREE) {
            merger->size += merger->next->size + sizeof(struct chunk);
            merger->next = merger->next->next;
        } else {
            merger = merger->next;//merger with the next chunk
        }
    }
}

void *calloc(size_t num, size_t size)
{
    char* p = (char*) malloc(num*size);
    if(p == NULL)
        return p;
    int k = 0;
    while (k < num*size)
    {
        p[k] = 0;
        ++k;
    }
    return p;
}

void *realloc(void *oldptr, size_t size)
{
    return NULL;
}


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
    ptr = (char*)ptr;
    ptr = ptr - sizeof(struct chunk);
    struct chunk *p = (struct chunk*)ptr;
    p->state = FREE;
    struct chunk *merger = NULL;
    merger = chunk_head;
    while(merger!= NULL && merger->next!= NULL) {//possible to merge with the next block
        if (merger->next->state == FREE && merger->state == FREE) {
            merger->size += merger->next->size + sizeof(struct chunk);
            merger->next = merger->next->next;
        } else {
            merger = merger->next;//merge with the next block
        }
    }
}

void *calloc(size_t num, size_t size)//allocate memory for an array and initialize it to zero
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
    //speacial case
    if (oldptr == NULL)
        return malloc(size);
    if (size == 0)
        free(oldptr);
        return NULL;
    //common case
    oldptr = (char*) oldptr;
    struct chunk* old_chunk = (struct chunk*)(oldptr - sizeof(struct chunk));
    if(old_chunk->size >= size) {//old chunk is bigger, clone the prevoius part
        char* newptr = (char*) malloc(size);
        if(newptr == NULL)
            return NULL;
        else {
            memcpy(newptr, oldptr, size);//choose a smaller size to copy
            free(oldptr);
            return newptr;
        }
        } else {//old chunk is smaller, allocate new block and copy the data
            void* newptr = malloc(size);
            if(newptr == NULL)
                return NULL;
            memcpy(newptr, oldptr, old_chunk->size);//choose a smaller part to copy
            free(oldptr);
            return newptr;
        }
    return NULL;
}

/*************D O  N O T  T O U C H  A N Y T H I N G  B E L O W*************/
static void tsk_malloc(void *pv)
{
  int i, c = (int)pv;
  char **a = malloc(c*sizeof(char *));
  for(i = 0; i < c; i++) {
	  a[i]=malloc(i+1);
	  a[i][i]=17;
  }
  for(i = 0; i < c; i++) {
	  free(a[i]);
  }
  free(a);

  task_exit(0);
}

#define MESSAGE(foo) printf("%s, line %d: %s", __FILE__, __LINE__, foo)
void test_allocator()
{
  char *p, *q, *t;

  MESSAGE("[1] Test malloc/free for unusual situations\r\n");

  MESSAGE("  [1.1]  Allocate small block ... ");
  p = malloc(17);
  if (p == NULL) {
    printf("FAILED\r\n");
	return;
  }
  p[0] = p[16] = 17;
  printf("PASSED\r\n");

  MESSAGE("  [1.2]  Allocate big block ... ");
  q = malloc(4711);
  if (q == NULL) {
    printf("FAILED\r\n");
	return;
  }
  q[4710] = 47;
  printf("PASSED\r\n");

  MESSAGE("  [1.3]  Free big block ... ");
  free(q);
  printf("PASSED\r\n");

  MESSAGE("  [1.4]  Free small block ... ");
  free(p);
  printf("PASSED\r\n");

  MESSAGE("  [1.5]  Allocate huge block ... ");
  q = malloc(32*1024*1024-sizeof(struct chunk));
  if (q == NULL) {
    printf("FAILED\r\n");
	return;
  }
  q[32*1024*1024-sizeof(struct chunk)-1]=17;
  free(q);
  printf("PASSED\r\n");

  MESSAGE("  [1.6]  Allocate zero bytes ... ");
  if ((p = malloc(0)) != NULL) {
    printf("FAILED\r\n");
	return;
  }
  printf("PASSED\r\n");

  MESSAGE("  [1.7]  Free NULL ... ");
  free(p);
  printf("PASSED\r\n");

  MESSAGE("  [1.8]  Free non-allocated-via-malloc block ... ");
  int arr[5] = {0x55aa4711,0x5a5a1147,0xa5a51471,0xaa551741,0x5aa54171};
  free(&arr[4]);
  if(arr[0] == 0x55aa4711 &&
     arr[1] == 0x5a5a1147 &&
	 arr[2] == 0xa5a51471 &&
	 arr[3] == 0xaa551741 &&
	 arr[4] == 0x5aa54171) {
	  printf("PASSED\r\n");
  } else {
	  printf("FAILED\r\n");
	  return;
  }

  MESSAGE("  [1.9]  Various allocation pattern ... ");
  int i;
  size_t pagesize = sysconf(_SC_PAGESIZE);
  for(i = 0; i < 7411; i++){
    p = malloc(pagesize);
	p[pagesize-1]=17;
    q = malloc(pagesize * 2 + 1);
	q[pagesize*2]=17;
    t = malloc(1);
	t[0]=17;
    free(p);
    free(q);
    free(t);
  }

  char **a = malloc(2741*sizeof(char *));
  for(i = 0; i < 2741; i++) {
	  a[i]=malloc(i+1);
	  a[i][i]=17;
  }
  for(i = 0; i < 2741; i++) {
	  free(a[i]);
  }
  free(a);

  if(chunk_head->next != NULL || chunk_head->size != 32*1024*1024) {
	printf("FAILED\r\n");
	return;
  }
  printf("PASSED\r\n");

  MESSAGE("  [1.10] Allocate using calloc ... ");
  int *x = calloc(17, 4);
  for(i = 0; i < 17; i++)
	  if(x[i] != 0) {
		  printf("FAILED\r\n");
		  return;
	  } else
	      x[i] = i;
  free(x);
  printf("PASSED\r\n");

  MESSAGE("[2] Test realloc() for unusual situations\r\n");

  MESSAGE("  [2.1]  Allocate 17 bytes by realloc(NULL, 17) ... ");
  p = realloc(NULL, 17);
  if (p == NULL) {
    printf("FAILED\r\n");
	return;
  }
  p[0] = p[16] = 17;
  printf("PASSED\r\n");
  MESSAGE("  [2.2]  Increase size by realloc(., 4711) ... ");
  p = realloc(p, 4711);
  if (p == NULL) {
    printf("FAILED\r\n");
	return;
  }
  if ( p[0] != 17 || p[16] != 17 ) {
    printf("FAILED\r\n");
	return;
  }
  p[4710] = 47;
  printf("PASSED\r\n");

  MESSAGE("  [2.3]  Decrease size by realloc(., 17) ... ");
  p = realloc(p, 17);
  if (p == NULL) {
    printf("FAILED\r\n");
	return;
  }
  if ( p[0] != 17 || p[16] != 17 ) {
    printf("FAILED\r\n");
	return;
  }
  printf("PASSED\r\n");

  MESSAGE("  [2.4]  Free block by realloc(., 0) ... ");
  p = realloc(p, 0);
  if (p != NULL) {
	printf("FAILED\r\n");
    return;
  } else
	printf("PASSED\r\n");

  MESSAGE("  [2.5]  Free block by realloc(NULL, 0) ... ");
  p = realloc(realloc(NULL, 0), 0);
  if (p != NULL) {
    printf("FAILED\r\n");
    return;
  } else
	printf("PASSED\r\n");

  MESSAGE("[3] Test malloc/free for thread-safe ... ");

  int t1, t2;
  char *s1 = malloc(1024*1024),
       *s2 = malloc(1024*1024);
  t1=task_create(s1+1024*1024, tsk_malloc, (void *)5000);
  t2=task_create(s2+1024*1024, tsk_malloc, (void *)5000);
  task_wait(t1, NULL);
  task_wait(t2, NULL);
  free(s1);
  free(s2);

  if(chunk_head->next != NULL || chunk_head->size != 32*1024*1024) {
	printf("FAILED\r\n");
	return;
  }
  printf("PASSED\r\n");
}
/*************D O  N O T  T O U C H  A N Y T H I N G  A B O V E*************/
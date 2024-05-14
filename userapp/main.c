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
#include<time.h>
#define RATIO 0.9

extern void* tlsf_create_with_pool(void* mem, size_t bytes);
extern void* g_heap;

/**
 * GCC insists on __main
 *    http://gcc.gnu.org/onlinedocs/gccint/Collect2.html
 */
void __main() 
{
    size_t heap_size = 32 * 1024 * 1024;
    void* heap_base = mmap(NULL, heap_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    g_heap = tlsf_create_with_pool(heap_base, heap_size);
}

struct window_info_t {
	int width;
	int height;
	int x;
	int y;
};
typedef struct window_info_t window_info;

struct control_arg {
	int tid_foo1;
	int tid_foo2;
};
//线程优先级

//函数原型
void bubsort1(int* a, int n);
void bubsort2(int* a, int n);
void tsk_foo1(void* pv);
void tsk_foo2(void* pv);
void render_array(COLORREF color, int* a, int n, window_info* window);
void render_bar(COLORREF color, int percent, short left_gravity, window_info window);
#define MATSIZE 4
typedef float matrix[MATSIZE * MATSIZE];
typedef float vector[MATSIZE];
//睡眠函数
void fsleep(int sec, int nsec) {
	struct timespec *tim = (struct timespec *)malloc(sizeof(struct timespec));
	tim->tv_sec = sec;
	tim->tv_nsec = nsec;
	nanosleep(tim, NULL);
	free(tim);
}

COLORREF color_l = RGB(66, 255, 255);
COLORREF color_r = RGB(99, 255, 88);
#define buffer_size 5
#define task4_ARRSIZE 535
static int mutex[buffer_size];
static int full;
static int empty;
static int arr[buffer_size][task4_ARRSIZE];
#define sleeptime 1
#define render_on
static window_info* window_p;
static window_info* window_c;
static window_info* window_ctrl;
static struct control_arg* cpv;
typedef  struct fadeColor
{
	float r;
	float g;
	float b;
} fadeColor;
fadeColor c1 = {(float) 0x00,(float) 0xFF,(float) 0xB3};
fadeColor c2 = {(float) 0xFF,(float) 0x7A,(float) 0xE4};


void consumer_thread(void* pv) {
	COLORREF black = RGB(0,0,0);
	int i, j, temp;
	int k = 0;
	while(1) {
		sem_wait(full);
		sem_wait(mutex[k]);
		window_c->x = window_c-> width* k;
		int render_y = 0;
		for (i = 0; i < task4_ARRSIZE; i++)
		{
			for (j = 0; j < task4_ARRSIZE - i - 1; j++)
			{
				if (arr[k][j] > arr[k][j + 1])
				{
					temp = arr[k][j];
					arr[k][j] = arr[k][j + 1];
					arr[k][j + 1] = temp;
				}
				while(render_y < task4_ARRSIZE){
					int length;
					length = arr[k][render_y];
					line(window_c->x, window_c->y + render_y, window_c->x + length, window_c-> y + render_y,RGB(
						(int)(c1.r + (c1.r - c2.r) * ((float)length/(float)window_p->width)),
						(int)(c1.g + (c1.g - c2.g) * ((float)length/(float)window_p->width)),
						(int)(c1.b + (c1.b - c2.b) * ((float)length/(float)window_p->width))));
					line(window_c->x + length, window_c-> y + render_y, window_c->x + window_c->width - 1, window_c-> y + render_y, black);
					render_y++;
				} 
			}
					render_y = 0;

		}
		sem_signal(mutex[k]);
		sem_signal(empty);
		++k;
		if (k == buffer_size) k = 0;

	}
}

void producer_thread(void* pv) {
	int k = 0;
	int i = 0;
	COLORREF black = RGB(0,0,0);

	srand(time(NULL));
	while(1) {
		int render_y = 0;
		sem_wait(empty);
		sem_wait(mutex[k]);
		window_p->x = k * window_p->width;
		for (i = 0; i < task4_ARRSIZE; i++)
		{
			arr[k][i] = rand() % (window_p->width);
			int length;
			length = arr[k][i];
			line(window_p->x, window_p->y + render_y, window_p->x + length, window_p-> y + render_y, RGB(
						(int)(c1.r + (c1.r - c2.r) * ((float)length/(float)window_c->width)),
						(int)(c1.g + (c1.g - c2.g) * ((float)length/(float)window_c->width)),
						(int)(c1.b + (c1.b - c2.b) * ((float)length/(float)window_c->width))));
			line(window_p->x + length, window_p-> y + render_y, window_p->x + window_p->width, window_p-> y + render_y, black);
			render_y++;
			// nanosleep((const struct timespec[]){{0,10000000L}}, NULL);
			fsleep(0,10000000L);
		}
		sem_signal(mutex[k]);
		sem_signal(full);
		++k;
		if (k == buffer_size) k = 0;
		// fsleep(sleeptime);


	}
	free(window_p);
	return;
}

void key_control() {
	int tid_foo1 = cpv->tid_foo1;
	int tid_foo2 = cpv->tid_foo2;
	int x;
	double percent_l = (double)((float)getpriority(tid_foo1) /39.0f);
	double percent_r = (double)((float)getpriority(tid_foo2) /39.0f);
	for (x = 0; x < window_ctrl->width * percent_l; x++)
	{
		line(x,window_ctrl->y,
			x,window_ctrl->y+window_ctrl->height/2,RGB(
				(int)(c2.r - (c2.r - c1.r) * ((float)x/(float)window_ctrl->width)),
				(int)(c2.g - (c2.g - c1.g) * ((float)x/(float)window_ctrl->width)),
				(int)(c2.b - (c2.b - c1.b) * ((float)x/(float)window_ctrl->width))));
	}
	for (; x < window_ctrl->width; x++)
	{
		line(x,window_ctrl->y,
		x,window_ctrl->y+window_ctrl->height/2,RGB(0,0,0));
	}

	for (x = 0; x < window_ctrl->width * percent_r; x++)
	{
		line(x,window_ctrl->y+window_ctrl->height/2 + 2,
			x,window_ctrl->y+window_ctrl->height,RGB(
				(int)(c1.r - (c1.r - c2.r) * ((float)x/(float)window_ctrl->width)),
				(int)(c1.g - (c1.g - c2.g) * ((float)x/(float)window_ctrl->width)),
				(int)(c1.b - (c1.b - c2.b) * ((float)x/(float)window_ctrl->width))));
	}
	for (; x < window_ctrl->width; x++)
	{
		line(x,window_ctrl->y+window_ctrl->height/2 + 2,
		x,window_ctrl->y+window_ctrl->height,RGB(0,0,0));
	}

	int mykeypress = 0x0;
	while (1) {
		mykeypress = getchar();
		switch (mykeypress)
		{
		case 0x4800://(up)
		{
		setpriority(tid_foo1, getpriority(tid_foo1) + 2);
		percent_l = (double)((float)getpriority(tid_foo1) / 39.0f);
		for (x = 0; x < window_ctrl->width * percent_l; x++)
		{
			line(x,window_ctrl->y,
				x,window_ctrl->y+window_ctrl->height/2,RGB(
					(int)(c2.r - (c2.r - c1.r) * ((float)x/(float)window_ctrl->width)),
					(int)(c2.g - (c2.g - c1.g) * ((float)x/(float)window_ctrl->width)),
					(int)(c2.b - (c2.b - c1.b) * ((float)x/(float)window_ctrl->width))));
		}
		for (; x < window_ctrl->width; x++)
		{
			line(x,window_ctrl->y,
			x,window_ctrl->y+window_ctrl->height/2,RGB(0,0,0));
		}
		}
		break;
		case 0x5000://(down)
		{
		setpriority(tid_foo1, getpriority(tid_foo1)-2);
		percent_l = (float)((float)getpriority(tid_foo1) /39.0f);
		for (x = 0; x < window_ctrl->width * percent_l; x++)
		{
			line(x,window_ctrl->y,
				x,window_ctrl->y+window_ctrl->height/2,RGB(
					(int)(c2.r - (int)(c2.r - c1.r) * ((float)x/(float)window_ctrl->width)),
					(int)(c2.g - (int)(c2.g - c1.g) * ((float)x/(float)window_ctrl->width)),
					(int)(c2.b - (int)(c2.b - c1.b) * ((float)x/(float)window_ctrl->width))));
		}
		for (; x < window_ctrl->width; x++)
		{
			line(x,window_ctrl->y,
			x,window_ctrl->y+window_ctrl->height/2,RGB(0,0,0));
		}
		}
		break;
//0x4d00(right)/0x4b00(left)
		case 0x4d00:
		{
		setpriority(tid_foo2, getpriority(tid_foo2)+2);
		percent_r = (float)((float)getpriority(tid_foo2) /39.0f);
		for (x = 0; x < window_ctrl->width * percent_r; x++)
		{
			line(x,window_ctrl->y+window_ctrl->height/ 2 + 2,
				x,window_ctrl->y+window_ctrl->height,RGB(
					(int)(c1.r - (int)(c1.r - c2.r) * ((float)x/(float)window_ctrl->width)),
					(int)(c1.g - (int)(c1.g - c2.g) * ((float)x/(float)window_ctrl->width)),
					(int)(c1.b - (int)(c1.b - c2.b) * ((float)x/(float)window_ctrl->width))));
		}
		for (; x < window_ctrl->width; x++)
		{
			line(x,window_ctrl->y+window_ctrl->height/2 + 2,
			x,window_ctrl->y+window_ctrl->height,RGB(0,0,0));
		}
		
		}
		break;
		case 0x4b00:
		{
		setpriority(tid_foo2, getpriority(tid_foo2)-2);
		percent_r = (float)((float)getpriority(tid_foo2) /39.0f);
		for (x = 0; x < window_ctrl->width * percent_r; x++)
		{
			line(x,window_ctrl->y+window_ctrl->height/2 + 2,
				x,window_ctrl->y+window_ctrl->height,RGB(
					(int)(c1.r - (c1.r - c2.r) * ((float)x/(float)window_ctrl->width)),
					(int)(c1.g - (c1.g - c2.g) * ((float)x/(float)window_ctrl->width)),
					(int)(c1.b - (c1.b - c2.b) * ((float)x/(float)window_ctrl->width))));
		}
		for (; x < window_ctrl->width; x++)
		{
			line(x,window_ctrl->y+window_ctrl->height/2 + 2,
			x,window_ctrl->y+window_ctrl->height,RGB(0,0,0));
		}
		}
		break;
		default:
			break;
		}
	}
}

void main(void *pv)
{


    printf("task #%d: I'm the first user task(pv=0x%08x)!\r\n",
            task_getid(), pv);
#ifdef render_on
	init_graphic(0x0115);
#endif
	window_p = (window_info*)malloc(sizeof(window_info));
	window_c = (window_info*)malloc(sizeof(window_info));
	window_ctrl = (window_info*)malloc(sizeof(window_info));

    window_p->width = (int)(g_graphic_dev.XResolution / buffer_size);
	window_p->height = (int)(g_graphic_dev.YResolution * 0.9);
	window_p->y = 0;
	window_p->x = 0;

	window_c->width = (int)(g_graphic_dev.XResolution / buffer_size);
	window_c->height = (int)(g_graphic_dev.YResolution * 0.9);
	window_c->y = 0;

	window_ctrl->y = (int)(g_graphic_dev.YResolution*0.9);
	window_ctrl->x = 0;
	window_ctrl->height = g_graphic_dev.YResolution - window_ctrl->y;
	window_ctrl->width = g_graphic_dev.XResolution;

	// printf("window_height %d",window_p->height);
	int i = 0;
	for (i = 0; i < buffer_size; i++)
	{
		mutex[i] = sem_create(1);
		// printf("-*mutex created, semid: %d*-\n\r",mutex[i]);
	}
	full = sem_create(0);
	// printf("-*full semid: %d*-\n\r",full);
	empty = sem_create(buffer_size);
	// printf("-*empty semid: %d*-\n\r",empty);
    while(1){
		int stack_consumer, stack_producer, stack_control;
		int stack_size_c = 11*1024*1024;
		int stack_size_p = 11*1024*1024;
		int stack_size_ctl = 8*1024*1024;
		int tid_consumer, tid_producer, tid_control;
		cpv = (struct control_arg*)(malloc(sizeof(struct control_arg)));//need to pass pv through a pointer
		stack_producer = (int)malloc(stack_size_c);
		stack_consumer = (int)malloc(stack_size_p);
		stack_control = (int)malloc(stack_size_ctl);

		char* _stack_producer = (char*)(stack_size_p +  stack_producer);
		char* _stack_consumer = (char*)(stack_size_c +  stack_consumer);
		char* _stack_control = (char*)(stack_size_ctl +  stack_control);

		printf("-*STACK INIT FINISHED*-\r\n");
		printf("-*STACK consumer %x*-\r\n", stack_consumer);
		printf("-*STACK producer %x*-\r\n", stack_producer);
		printf("-*STACK control %x*-\r\n", stack_control);

		void* p = NULL;
		tid_producer = task_create(_stack_producer, producer_thread, p);
		tid_consumer = task_create(_stack_consumer, consumer_thread, p);
		cpv->tid_foo1 = tid_consumer;
		cpv->tid_foo2 = tid_producer;

		tid_control = task_create(_stack_control, key_control, p);
		setpriority(tid_producer, 10);
		setpriority(tid_consumer, 10);
		setpriority(tid_control, 0);

		// printf("-*control PID %x*-\r\n", tid_control);
		// printf("-*consumer PID %x*-\r\n", tid_consumer);
		// printf("-*producer PID %x*-\r\n", tid_producer);
		// printf("-*CONSUMER CREATED ID = %d*-\r\n", tid_consumer);



		// printf("-*KEY_CTRL CREATED ID = %d*-\r\n", tid_control);


		fsleep(32767,0);
		//why I'm doing this? fuck you
		// free((void*)stack_consumer);
		// free((void*)stack_producer);
		// free((void*)stack_control);
		// free((void*)window_c);
		// free((void*)window_p);
    };
    task_exit(0);
}
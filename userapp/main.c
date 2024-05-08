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
void key_control(void* pv);
void render_array(COLORREF color, int* a, int n, window_info window);
void render_bar(COLORREF color, int percent, short left_gravity, window_info window);
void mySleep();
#define MATSIZE 4
typedef float matrix[MATSIZE * MATSIZE];
typedef float vector[MATSIZE];
//睡眠函数
void fsleep(float sec) {
	struct timespec tim, tim2;
	int isec = (int)sec;
	int nsec = (int)(1000000000 * (sec - isec));
	tim.tv_sec = isec;
	tim.tv_nsec = nsec;
	nanosleep(&tim, &tim2);
}


COLORREF color_l = RGB(66, 255, 255);
COLORREF color_r = RGB(99, 255, 88);
// //冒泡排序
// void bubsort1(int* a, int n){
// 	int i, j, temp;
// 	for (i = 0; i < n; i++)
// 	{
// 		for (j = 0; j < n - i - 1; j++)
// 		{
// 			if (a[j] > a[j + 1])
// 			{
// 				temp = a[j];
// 				a[j] = a[j + 1];
// 				a[j + 1] = temp;
// 				window_info window1 = {710, 850, 0, 0};
// 				render_array(color_r, a, n, window1);
// 			}
// 			//mySleep();
// 		}
// 		mySleep();
// 	}
// }

// void bubsort2(int* a, int n)
// {
// 	int i, j, temp;
// 	for (i = 0; i < n; i++)
// 	{
// 		for (j = 0; j < n - i - 1; j++)
// 		{
// 			if (a[j] > a[j + 1])
// 			{
// 				temp = a[j];
// 				a[j] = a[j + 1];
// 				a[j + 1] = temp;
// 				window_info window2 = {710, 850, 720, 0};
// 				render_array(color_l, a, n, window2);
// 			}
// 		}
// 		mySleep();
// 	}
// }
// #define ARRSIZE 50
// //线程函数
// void tsk_foo1(void* pv)
// {
// 	time_t time(time_t * loc);
// 	srand(time(NULL));
// 	int myCount_1[ARRSIZE];
// 	int i;
// 	for (i = 0; i < ARRSIZE; i++)
// 	{
// 		myCount_1[i] = rand() % 200;
// 		printf("%d\n", myCount_1[i]);
// 	}
// 	window_info window = {710, 850, 0, 0};
// 	render_array(color_l, myCount_1, ARRSIZE, window);
// 	mySleep();
// 	bubsort1(myCount_1, ARRSIZE);
// 	task_exit(0);
// }

// void tsk_foo2(void* pv)
// {
// 	time_t time(time_t * loc);
// 	srand(time(NULL));
// 	int myCount_2[ARRSIZE];
// 	int i;
// 	for (i = 0; i < ARRSIZE; i++)
// 	{
// 		myCount_2[i] = rand() % 200;
// 		printf("%d\n", myCount_2[i]);
// 	}
// 	//显示未排序的画面
// 	window_info window = {710, 850, 720, 0};
// 	render_array(color_r, myCount_2, ARRSIZE, window);
// 	mySleep();
// 	bubsort2(myCount_2, ARRSIZE);
// 	task_exit(0);
// }

//控制线程
void key_control(void* pv) {
#define LEFT_GRAV 1
#define RIGHT_GRAV 2
	struct control_arg* px = (struct control_arg*) pv;
	int tid_foo1 = px->tid_foo1;
	int tid_foo2 = px->tid_foo2;
	int percent_l = (int)(getpriority(tid_foo1) * 100 / 127);
	int percent_r = (int)(getpriority(tid_foo2) * 100 / 127);
	window_info window_l;
	window_info window_r;
	//The bar may be a little fat..
	window_r.width = (int)(g_graphic_dev.XResolution / 2);
	window_l.height = (int)(g_graphic_dev.YResolution * 0.18);//coopreate with another part, may see consumer and producer thread
	window_r.x = (int)(g_graphic_dev.XResolution * 0.5);
	window_l.x = 0;
	window_r.y = (int)(g_graphic_dev.YResolution * 0.80);
	window_l.y = (int)(g_graphic_dev.YResolution * 0.80);

	render_bar(color_r, percent_l, RIGHT_GRAV, window_l);
	render_bar(color_l, percent_r, LEFT_GRAV, window_r);

	int mykeypress = 0x0;
	while (1) {
		mykeypress = getchar();
		switch (mykeypress)
		{
		case 0x4800://(up)
		{
			setpriority(tid_foo1, getpriority(tid_foo1) + 1);
			render_bar(color_r, getpriority(tid_foo1), RIGHT_GRAV, window_l);
		}
		break;
		case 0x5000://(down)
		{
			setpriority(tid_foo1, getpriority(tid_foo1)-1);
			render_bar(color_r, getpriority(tid_foo1), RIGHT_GRAV, window_l);
		}
		break;
//0x4d00(right)/0x4b00(left)
		case 0x4d00:
		{
			setpriority(tid_foo2, getpriority(tid_foo2) + 1);
			render_bar(color_l, getpriority(tid_foo2), LEFT_GRAV, window_r);
		}
		break;
		case 0x4b00:
		{
			setpriority(tid_foo2, getpriority(tid_foo2) - 1);
			render_bar(color_l, getpriority(tid_foo2), LEFT_GRAV, window_r);
		}
		break;
		default:
			break;
		}
	}
}

void render_array(COLORREF color, int* arr, int size, window_info windowInfo) {
	int  X_pos, X_size, Y_pos;
	X_pos = windowInfo.x;
	X_size = windowInfo.width;
	Y_pos = windowInfo.y;

    // int y_reso = g_graphic_dev.YResolution;
	int y_reso = windowInfo.height;
    int j = 0;
    int max = 0;
    int min = 100;
    float ratio = RATIO;
    COLORREF default_black = RGB(0, 0, 0);
    for (; j < size; j++)
    {
        if (arr[j] > max)
        {
            max = arr[j];
        } else if (arr[j] < min)
        {
            min = arr[j];
        }
    }
    j = 0;
    int y_step = (int) y_reso / size;
    for (; j < size; j++)
    {	
        int y = Y_pos + y_step * j;
        float temp = (arr[j] * X_size)/ max;
        int X_len = (int) temp; 
		COLORREF fade_color = color + (int)((float)X_len / 5);
        int i = 0;
        temp = ratio * y_step;
        int render_thickness = (int) temp;
        for (; i < render_thickness; i++)
        {
            line(X_pos, y + i, X_pos + X_len, y + i, fade_color);
            line(X_pos + X_len, y + i, X_pos + X_size, y + i, default_black);
        }
    }
}

void render_bar(COLORREF color, int percent, short left_gravity, window_info windowInfo) {
	float percent_ = (float)percent / 45; //magic number, do not change unless piority system is changed
	color = color + 4*percent;
	if (left_gravity == 1){
		size_t y = windowInfo.y;
		for (; y < windowInfo.y + windowInfo.height; y++)
			{
				line(windowInfo.x, y, (int)(windowInfo.x + windowInfo.width * percent_), y, color);
				line((int)(windowInfo.x + windowInfo.width * percent_), y, windowInfo.x + windowInfo.width, y, RGB(0, 0, 0));
			}
	} else {
		size_t y = windowInfo.y;
		for (; y < windowInfo.y + windowInfo.height; y++)
			{
				line(windowInfo.x + windowInfo.width, y, (int)(windowInfo.x + windowInfo.width - windowInfo.width * percent_), y, color);
				line((int)(windowInfo.x + windowInfo.width - windowInfo.width * percent_), y, windowInfo.x, y, RGB(0, 0, 0));
			}
	}
	return;
}

#define buffer_size 10
#define task4_ARRSIZE 50
static int mutex[buffer_size];
static int full;
static int empty;
static int arr[buffer_size][task4_ARRSIZE];
#define sleeptime 2

void consumer_bub(int* a, int n, int k)
{
	window_info window;
	window.width = (int)(g_graphic_dev.XResolution / buffer_size);
	window.height = (int)(g_graphic_dev.YResolution * 0.9);
	window.y = 0;
	window.x = (int)(g_graphic_dev.XResolution / buffer_size) * k;
	int i, j, temp;
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < n - i - 1; j++)
		{
			if (a[j] > a[j + 1])
			{
				temp = a[j];
				a[j] = a[j + 1];
				a[j + 1] = temp;
				// render_array(color_r, a, n, window);	
			}
		}
	}
}

void consumer_thread(void* pv) {
	int k = 0;
	int i = 0;
	printf("ENTER CONSUMER\n\r");
	while(1) {
		sem_wait(full);
		sem_wait(mutex[k]);
		for ( i = 0; i < buffer_size; i++)
		{
			consumer_bub(arr[k], task4_ARRSIZE, k);
		}
		sem_signal(mutex[k]);
		sem_signal(empty);
		fsleep(sleeptime);
		printf("Consumer Check\n\r");
		++k;
		if (k == buffer_size) k = 0;
	}

}

void producer_thread(void* pv) {
	int k = 0;
	int i = 0;
	printf("ENTER PRODUCER\n\r");
	window_info window;
	window.width = (int)(g_graphic_dev.XResolution / buffer_size) - 1;
	window.height = (int)(g_graphic_dev.YResolution * 0.9);
	window.y = 0;
	window.x = (int)(g_graphic_dev.XResolution / buffer_size) * k;
	srand(time(NULL));
	while(1) {
		sem_wait(empty);
		printf("producer get empty\n\r");
		sem_wait(mutex[k]);
		printf("producer get mutex\n\r");
		for (i = 0; i < task4_ARRSIZE; i++)
		{
			arr[k][i] = rand() % (4 * task4_ARRSIZE);
			// render_array(color_l, arr[k], task4_ARRSIZE, window);
		}
		printf("Producer Check\n\t");
		sem_signal(mutex[k]);
		sem_signal(full);
		fsleep(sleeptime);
		++k;
		if (k == buffer_size) k = 0;
	}
}


void main(void *pv)
{
    
    printf("task #%d: I'm the first user task(pv=0x%08x)!\r\n",
            task_getid(), pv);
	// init_graphic(0x115);
	int i = 0;
	for (i = 0; i < buffer_size; i++)
	{
		mutex[i] = sem_create(1);
		printf("-*mutex created, semid: %d*-\n\r",mutex[i]);
	}
	full = sem_create(0);
	printf("-*full semid: %d*-\n\r",full);
	empty = sem_create(buffer_size);
	printf("-*empty semid: %d*-\n\r",empty);
    while(1){
		unsigned char* stack_consumer, stack_producer, stack_control;
		unsigned int stack_size = 4096*4096;
		int tid_consumer, tid_producer, tid_control;
		struct control_arg control_pv;
		struct control_arg* cpv = &control_pv;//need to pass pv through a pointer
		stack_producer = (unsigned char*)malloc(stack_size);
		stack_consumer = (unsigned char*)malloc(stack_size);

		// stack_control = (unsigned char*)malloc(stack_size);

		printf("-*STACK INIT FINISHED*-\r\n");
		printf("-*STACK consumer %x*-\r\n", stack_consumer);
		printf("-*STACK producer %x*-\r\n", stack_producer);
		// printf("-*STACK control %x*-\r\n", stack_control);


		tid_producer = task_create(stack_producer + stack_size, &producer_thread, NULL);
		tid_consumer = task_create(stack_consumer + stack_size, &consumer_thread, NULL);

		printf("-*PRODUCER CREATED ID = %d*-\r\n", tid_producer);
		printf("-*CONSUMER CREATED ID = %d*-\r\n", tid_consumer);

		control_pv.tid_foo1 = tid_consumer;
		control_pv.tid_foo2 = tid_producer;
		// tid_control = task_create(stack_control + stack_size, &key_control, cpv);
		// printf("-*KEY_CTRL CREATED ID = %d*-\r\n", tid_control);

		setpriority(tid_producer, 10);
		setpriority(tid_consumer, 10);
		// setpriority(tid_control, 0);
		//why I'm doing this? fuck you
		fsleep(120.0);
		// free(stack_consumer);
		// free(stack_producer);
		// free(stack_control);
    };
    task_exit(0);
}




// void task3(void* pv){
// 	unsigned char* stack_foo_1, * stack_foo_2, * stack_foo_3;
// 	unsigned int  stack_size = 1024 * 1024;
// 	stack_foo_1 = (unsigned char*)malloc(stack_size);
// 	stack_foo_2 = (unsigned char*)malloc(stack_size);
// 	stack_foo_3 = (unsigned char*)malloc(stack_size);
// 	init_graphic(0x017f);

// 	tid_foo1 = task_create(stack_foo_1 + stack_size, &tsk_foo1, (void*)0);
// 	tid_foo2 = task_create(stack_foo_2 + stack_size, &tsk_foo2, (void*)0);
// 	tid_foo3 = task_create(stack_foo_3 + stack_size, &key_control, (void*)0);
// 	setpriority(tid_foo3, 0);
// 	setpriority(tid_foo1, 10);
// 	setpriority(tid_foo2, 10);

// 	free(stack_foo_1);
// 	free(stack_foo_2);
// 	free(stack_foo_3);

// 	while (1)
// 		;
// 	task_exit(0);
// }

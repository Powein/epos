/**
 * vim: filetype=c:fenc=utf-8:ts=4:et:sw=4:sts=4
 */
#include <stddef.h>
#include "kernel.h"
#define SEM_debug
struct Semaphore* sem_head = NULL;
static uint32_t semid = 0;
struct Semaphore* find_semamphore(int semid) {
    struct Semaphore* temp = sem_head;
    if(!temp) return;
    if (temp -> sem_ID == semid) {
        return temp;
    }
    while(temp -> next_semaphore != NULL) {
        //check temp now
        temp = temp -> next_semaphore;
        if (temp -> sem_ID == semid) {
            //found target
            return temp;
        }
    }
    return NULL;
}

int sys_sem_create(int value)
{
    semid++;
    struct Semaphore* new_sem = (struct Semaphore*)kmalloc(sizeof(struct Semaphore));
    new_sem -> sem_ID = semid;
    new_sem -> value = value;
    new_sem -> wait_que = NULL;
    new_sem -> next_semaphore = NULL;
    struct Semaphore* temp = sem_head;
    if (new_sem) {
        if (sem_head == NULL) {
        //set it as head
            sem_head = new_sem;
        } else {
        //hang it on the tail
            while(temp -> next_semaphore!= NULL) {
                temp = temp -> next_semaphore;
            }
            temp -> next_semaphore = new_sem;
        }
        return new_sem->sem_ID;
    }
    else
        return -1;
    
}

int sys_sem_destroy(int semid)
{
    
    if(sem_head != NULL) {
        struct Semaphore* temp = sem_head;
        struct Semaphore* q;
        if (temp -> sem_ID == semid) {
            sem_head = sem_head -> next_semaphore;
            kfree(temp);
        }
        while(temp -> next_semaphore != NULL) {
            //q is checked
            q = temp;
            //check temp now
            temp = temp -> next_semaphore;
            if (temp -> sem_ID == semid) {
                //found target
                //delete temp
                q -> next_semaphore = temp -> next_semaphore;
                kfree(temp);
                break;
            }
        }
    } else
        return -1;
}

int sys_sem_wait(int semid)
{
    struct Semaphore* sem = (semid);
    sem = find_semamphore(semid);
#ifdef SEM_debug
    printk("semid %d is waiting, after wait, value is %d\n\r", sem->sem_ID, sem->value - 1);
#endif
    if(!sem) return -1;
    uint32_t flags;
    //The waitq is const
    //There is no need to lock it
    save_flags_cli(flags);
    sem->value--;
    if (sem -> value < 0) {
    //when sem<0, need to block current task
        sleep_on(&sem -> wait_que);
    }
    restore_flags(flags);
    return 0;
}

int sys_sem_signal(int semid)
{
    struct Semaphore* sem = find_semamphore(semid);
#ifdef SEM_debug
    printk("semid %d is signaling, after signal, value is %d\n\r", sem->sem_ID, sem->value + 1);
#endif
    if(sem == NULL) return -1;
    uint32_t flags;
    save_flags_cli(flags);
    sem->value++;
    if (sem -> value <= 0)
    //when the sem<=0, there must be >=1 blocked task
    //wake one task up
        wake_up(&sem-> wait_que, 1);
    restore_flags(flags);
    return 0;
}


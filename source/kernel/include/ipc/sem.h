#ifndef SEM_H
#define SEM_H

// Semaphore 信号量
// Inter-Process Communication 进程间通信

#include "tools/list.h"
#include "core/task.h"

typedef struct _sem_t {
    int count;
    list_t wait_list;
} sem_t;

extern void sem_init(sem_t *sem, int init_count);

extern void sem_wait(sem_t *sem);

extern void sem_notify(sem_t *sem);

extern int sem_count(sem_t *sem);

#endif
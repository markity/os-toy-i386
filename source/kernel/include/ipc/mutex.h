#ifndef MUTEX_H
#define MUTEX_H

#include "tools/list.h"
#include "core/task.h"

// 实现一个可重入互斥锁

typedef struct _rein_mutex_t {
    task_t *owner;
    int locker;
    list_t wait_list;
} rein_mutex_t;

extern void rein_mutex_init(rein_mutex_t *lock);

extern void rein_mutex_lock(rein_mutex_t *lock);
extern void rein_mutex_unlock(rein_mutex_t *lock);

#endif
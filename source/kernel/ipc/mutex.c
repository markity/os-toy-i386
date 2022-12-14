#include "ipc/mutex.h"

void rein_mutex_init(rein_mutex_t *lock) {
    lock->locker = 0;
    lock->owner = (task_t*)0;
    list_init(&lock->wait_list);
}

void rein_mutex_lock(rein_mutex_t *lock) {
    irq_state_t state = irq_enter_protection();

    task_t *curr_task = task_current();


    // 是否增加计数
    if(lock->locker == 0) {
        lock->locker ++;
        lock->owner = curr_task;
    } else if (lock->owner == curr_task) {
        lock->locker ++;
    } else {
        // 进行调度
        task_set_block(curr_task);
        list_insert_last(&lock->wait_list, &curr_task->wait_node);
        task_dispatch();
    }

    irq_leave_protection(state);
}

void rein_mutex_unlock(rein_mutex_t *lock) {
    irq_state_t state = irq_enter_protection();

    task_t *curr_task = task_current();

    // 避免写出奇怪的代码, 此处进行快退出检查
    ASSERT(lock->locker != 0);
    ASSERT(lock->owner == curr_task);

    if(lock->owner == curr_task) {
        if(--lock->locker == 0) {
            lock->owner = (task_t*)0;

            if(list_count(&lock->wait_list)) {
                list_node_t *node_new_owner = list_pop_first(&lock->wait_list);
                task_t *new_owner = CONTAINER_OF(node_new_owner, task_t, wait_node);
                task_set_ready(new_owner);
                lock->owner = new_owner;
                lock->locker ++;
                // TODO 这里为什么要dispatch
                // task_dispatch()
            }
        }
    }

    // locker不为0, 只递减locker

    irq_leave_protection(state);
}
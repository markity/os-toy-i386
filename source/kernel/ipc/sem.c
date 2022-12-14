#include "ipc/sem.h"

void sem_init(sem_t *sem, int init_count) {
    sem->count = init_count;
    list_init(&sem->wait_list);
}


extern void sem_wait(sem_t *sem) {
    irq_state_t state = irq_enter_protection();
    if(sem->count > 0) {
        sem->count --;
    } else {
        task_t *curr = task_current();
        // 从就绪队列中移除
        task_set_block(curr);
        // 加入到等待队列尾部
        list_insert_last(&sem->wait_list, &curr->wait_node);
        task_dispatch();
    }

    irq_leave_protection(state);
}

extern void sem_notify(sem_t *sem) {
    irq_state_t state = irq_enter_protection();

    if (list_count(&sem->wait_list)) {
        list_node_t *node = list_pop_first(&sem->wait_list);
        task_t *task = CONTAINER_OF(node, task_t, wait_node);
        task_set_ready(task);

        task_dispatch();
    } else {
        // 发了信号, 但是没有人接收
        sem->count ++;
    }

    irq_leave_protection(state);
}

int sem_count(sem_t *sem) {
    irq_state_t state = irq_enter_protection();
    int cnt = sem->count;
    irq_leave_protection(state);

    return cnt;
}

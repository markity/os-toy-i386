#include "core/task.h"

// 全局的任务管理器
task_manager_t task_manager;

static uint32_t idle_task_stack[IDLE_TASK_STACK_SIZE];

static void idle_task_entry(void) {
    for(;;) {
        hlt();
    }
}

static int tss_init(task_t *task, uint32_t entry, uint32_t esp) {
    int tss_sel = get_alloc_desc();
    if (tss_sel < 0) {
        return -1;
    }

    segment_desc_set(tss_sel, (uint32_t)(&task->tss), sizeof(tss_t),
        SEG_P_PRESENT | SEG_DPL_0 | SEG_TYPE_TSS
    );


    kernel_memset(&task->tss, 0, sizeof(tss_t));
    task->tss.eip = entry;

    /* esp0代表特权级0 */
    task->tss.esp = task->tss.esp0 = esp;

    /* 0同样代表特权级0 */
    task->tss.ss = task->tss.ss0 = KERNEL_SELECTOR_DATA;

    task->tss.es = task->tss.ds = task->tss.fs = task->tss.gs = KERNEL_SELECTOR_DATA;
    task->tss.cs = KERNEL_SELECTOR_CODE;  

    task->tss.eflags = ELFAGS_IF | EFLAGS_DEFAULT;

    task->tss_sel = tss_sel;
    return 0;
}

int task_init(task_t *task, const char *task_name, uint32_t entry, uint32_t esp) {
    tss_init(task, entry, esp);

    // 名字不对, 应当返回-1, 表示初始化task失败
    int task_name_length = kernel_strlen(task_name);
    if (task_name_length == 0 || task_name_length > TASK_NAME_MAX_IN_SIZE) {
        return -1;
    }
    kernel_strcpy(task->task_name, task_name);

    task->state = TASK_CREATED;
    task->sleep_ticks = 0;
    task->time_ticks = TASK_TIME_SLICE_DEFAULT;
    task->slice_ticks = task->time_ticks;


    list_node_init(&task->all_node);
    list_node_init(&task->run_node);
    list_node_init(&task->wait_node);

    if(task != &task_manager.idle_task) {
        irq_state_t state = irq_enter_protection();
        task_set_ready(task);
        list_insert_last(&task_manager.task_list, &task->all_node);
        irq_leave_protection(state);
    }

    return 0;
    

    // uint32_t *pesp = (uint32_t*)esp;
    // // 特判,如果esp = 0那么则为from任务
    // if (pesp) {
    //     *(--pesp) = entry;
    //     *(--pesp) = 0;
    //     *(--pesp) = 0;
    //     *(--pesp) = 0;
    //     *(--pesp) = 0;
    //     task->stack = pesp;
    // }
}

void task_switch_from_to(task_t *from, task_t *to) {
    // simple_switch(&from->stack, to->stack);
    switch_to_tss(to->tss_sel);
}

void task_manager_init() {
    list_init(&task_manager.ready_list);
    list_init(&task_manager.task_list);
    list_init(&task_manager.sleep_list);
    task_manager.curr_task = (task_t*)0;

    task_init(&task_manager.idle_task, IDLE_TASK_NAME, (uint32_t)idle_task_entry, (uint32_t)idle_task_stack+IDLE_TASK_STACK_SIZE);
}

void task_main_init(void) {
    task_init(&task_manager.main_task, "main", 0, 0);

    write_tr(task_manager.main_task.tss_sel);
    task_manager.curr_task = &task_manager.main_task;
}

task_t *task_main_task(void) {
    return &task_manager.main_task;
}

void task_set_ready(task_t *task) {
    list_insert_last(&task_manager.ready_list, &task->run_node);
    task->state = TASK_READY;
}

void task_set_block(task_t *task) {
    list_pop(&task_manager.ready_list, &task->run_node);
    // 可能处于等待状态或睡眠状态, 不确定状态, 这里不设置
}

task_t *task_current(void) {
    return task_manager.curr_task;
}

void sys_sched_yield(void) {
    irq_state_t state = irq_enter_protection();

    if(list_count(&task_manager.ready_list) > 1) {
        // 从头部移到尾部
        task_t *curr_task = task_current();
        task_set_block(curr_task);
        task_set_ready(curr_task);

        task_dispatch();
    }

    irq_leave_protection(state);
}


void task_dispatch(void) {
    irq_state_t state = irq_enter_protection();
    
    list_node_t *node = list_first(&task_manager.ready_list);
    task_t *from = task_current();

    if(node == (list_node_t*)0) {
        task_t *to = &task_manager.idle_task;
        if(to != from) {
            task_manager.curr_task = &task_manager.idle_task;
            task_switch_from_to(from, &task_manager.idle_task);
        }
    } else {
        task_t *to = CONTAINER_OF(node, task_t, run_node);
        if(to != from) {
            task_manager.curr_task = to;
            to->state = TASK_RUNNING;
            task_switch_from_to(from, to);
        }
    }
    irq_leave_protection(state);
}

void task_time_tick(void) {
    task_t *curr = task_current();
    if(curr != &task_manager.idle_task && --curr->slice_ticks == 0) {
        curr->slice_ticks = curr->time_ticks;
        // 移动到尾部
        task_set_block(curr);
        task_set_ready(curr);
    }


    list_node_t *curr_node = list_first(&task_manager.sleep_list);
    while(curr_node) {
        list_node_t *next_node = curr_node->next;
        task_t *task = CONTAINER_OF(curr_node, task_t, run_node);
        if(--task->sleep_ticks == 0) {
            // 从睡眠队列中移除
            task_set_wakeup(task);

            task_set_ready(task);
        }

        curr_node = next_node;
    }

    task_dispatch();
}

void task_set_sleep(task_t *task, uint32_t ticks) {
    if(ticks == 0) {
        return;
    }

    task->sleep_ticks = ticks;
    task->state = TASK_SLEEP;
    list_insert_last(&task_manager.sleep_list, &task->run_node);
}

void task_set_wakeup(task_t *task) {
    list_pop(&task_manager.sleep_list, &task->run_node);
}

void sys_sleep(uint32_t ms) {
    irq_state_t state = irq_enter_protection();

    task_t *curr = task_manager.curr_task;
    task_set_block(curr);
    // 向上取整
    task_set_sleep(curr, (ms + OS_TICKS_MS-1) /OS_TICKS_MS + 1);

    task_dispatch();
    irq_leave_protection(state);
}

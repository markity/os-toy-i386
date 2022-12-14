#ifndef TASK_H
#define TASK_H

#include "comm/types.h"
#include "cpu/cpu.h"
#include "tools/klib.h"
#include "tools/list.h"
#include "cpu/irq.h"

#define TASK_NAME_MAX_IN_SIZE 32
// 100ms的最大时间
#define TASK_TIME_SLICE_DEFAULT 10;

typedef enum _task_state_t {
    TASK_CREATED,
    TASK_RUNNING,
    TASK_SLEEP,
    TASK_READY,
    TASK_WAITTING,
} task_state;

typedef struct _task_t {
    // uint32_t *stack;

    char task_name[TASK_NAME_MAX_IN_SIZE+1];

    // 插入到ready_list
    list_node_t run_node;
    // 插入到task_list
    list_node_t all_node;
    // 插入到sem的wait_list
    list_node_t wait_node;

    task_state state;

    // 需要递减
    int slice_ticks;

    // 一个任务占用最大tick
    int time_ticks;

    // 延时时间片, 需要递减
    int sleep_ticks;

    tss_t tss;
    int tss_sel;
} task_t;

typedef struct _task_manager_t {
    // 就绪
    list_t ready_list;

    // 所有进程
    list_t task_list;

    // 睡眠队列
    list_t sleep_list;

    // 当前进程
    task_t *curr_task;

    // 最初程序的进程
    task_t main_task;

    // 空闲进程
    task_t idle_task;
} task_manager_t;

extern void task_manager_init(void);
extern void task_main_init(void);

extern task_t *task_main_task(void);
extern task_t *task_current(void);

extern int task_init(task_t *task, const char *task_name, uint32_t entry, uint32_t esp);
extern void task_switch_from_to(task_t *from, task_t *to);
/* 外部汇编函数, 细看i386调用规则c convention生成的汇编也符合这个调用规则

All registers on the Intel386 are global and thus visible to both a calling and a called function.
Registers %ebp, %ebx, %edi, %esi, and %esp ‘‘belong’’ to the calling function. In other words,
a called function must preserve these registers’ values for its caller. Remaining registers ‘‘belong’’
to the called function. If a calling function wants to preserve such a register value across a function call,
it must save the value in its local stack frame.
 */
// extern void simple_switch(uint32_t **from_stack, uint32_t *to_stack);

extern void task_set_ready(task_t *task);

extern void task_set_block(task_t *task);

extern void task_set_sleep(task_t *task, uint32_t ticks);

extern void task_set_wakeup(task_t *task);

extern void sys_sched_yield(void);

extern void task_dispatch(void);

extern void task_time_tick(void);

extern void sys_sleep(uint32_t ms);

#endif
#ifndef OS_CFG_H
#define OS_CFG_H

#define OS_VERSION "1.0"

/* 假设最多支持256张表, 其实可支持更多, 但是有限制 */
#define GDT_TABLE_SIZE          256

/* 内核的选择子 */
#define KERNEL_SELECTOR_CODE    (1*8)
#define KERNEL_SELECTOR_DATA    (2*8)

/* 内核栈空间大小 */
#define KERNEL_STACK_SIZE       (8*1024)

/* 10ms产生一次定时中断 */
#define OS_TICKS_MS             10

/* 空闲进程的栈大小 */
#define IDLE_TASK_STACK_SIZE    1024

#define IDLE_TASK_NAME          "idle_task"


#endif
#ifndef TIMER_H
#define TIMER_H

#include "comm/types.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "core/task.h"

// 时钟源, 一个常数
#define PIT_OSC_FREQ                1193182

// 定时器0的数据端口
#define PIT_CHANNEL0_DATA_PORT       0x40

// 定时器的模式和命令端口
#define PIT_COMMAND_MODE_PORT        0x43

// 指定配置0号定时器
#define PIT_CHANNLE0                (0 << 6)

// 一些需要被置位的位, 现在不管, 只需要知道, 如此配置, 会定时产生合适的中断就行了
#define PIT_LOAD_LOHI               (3 << 4)
#define PIT_MODE3                   (3 << 1)

extern void timer_init (void);
extern void exception_handler_timer (void);


#endif
#include "dev/timer.h"

// 系统时钟
static uint32_t sys_tick;

void do_handler_timer(exception_frame_t *frame) {
    sys_tick++;
    pic_send_eoi(IRQ0_TIMER);

    task_time_tick();
}


// 有三个定时器, 定时器0连接到IRQ0
static void init_pit (void) {
    // 时钟源表示1s震动多少次
    uint32_t reload_count = PIT_OSC_FREQ * OS_TICKS_MS / 1000;

    outb(PIT_COMMAND_MODE_PORT, PIT_CHANNLE0 | PIT_LOAD_LOHI | PIT_MODE3);

    // 设置计数值, 先写低8位, 再写高8位
    outb(PIT_CHANNEL0_DATA_PORT, reload_count & 0xFF);   // 加载低8位
    outb(PIT_CHANNEL0_DATA_PORT, (reload_count >> 8) & 0xFF); // 再加载高8位

    irq_install(IRQ0_TIMER, (uint32_t)exception_handler_timer);
    irq_enable(IRQ0_TIMER);
}

void timer_init(void) {
    sys_tick = 0;
    init_pit();
}

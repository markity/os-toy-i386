#include "tools/log.h"
#include "ipc/mutex.h"

/* 实现系统级日志打印, 方便打印加载系统的信息 */

rein_mutex_t mutex;


// 暂时使用串口, 现在不关心到底怎么配置的
#define COM1_PORT 0x3F8

void log_init() {
    rein_mutex_init(&mutex);
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x80);
    outb(COM1_PORT + 0, 0x3);
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x03);
    outb(COM1_PORT + 2, 0xc7);
    outb(COM1_PORT + 4, 0X0F);
}


void log_println(const char* msg) {
    rein_mutex_lock(&mutex);
    const char *p = msg;
    while(*p != '\0') {
        /* 忙 */
        while (inb(COM1_PORT + 5) & (1<<6) == 0) {}
        outb(COM1_PORT, *p++);
    }
    outb(COM1_PORT, '\r');
    outb(COM1_PORT, '\n');
    rein_mutex_unlock(&mutex);
}

/* 
我们现在不能控制屏幕, 现在用串口显示, serial0
这里我们来看看可变参数的使用, 实现printf的传参
 */
void log_printfln(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);


    /* 最大支持127个字符的输出 */
    char str_buf[128];

    kernel_vsprintf(str_buf, fmt, args);


    const char *p = str_buf;
    rein_mutex_lock(&mutex);
    while(*p != '\0') {
        /* 忙 */
        while (inb(COM1_PORT + 5) & (1<<6) == 0) {}
        outb(COM1_PORT, *p++);
    }
    outb(COM1_PORT, '\r');
    outb(COM1_PORT, '\n');
    rein_mutex_unlock(&mutex);

    va_end(args);
}
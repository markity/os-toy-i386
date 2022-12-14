/* 这个文件存放一些简单的cpu指令(instruction), 这些指令会被反复用到, 因此在此封装
    由于16位代码和32位代码不同, 因此将代码写在头文件中
    为了提高效率, 采用内联
 */

#ifndef _CPUINSTR_H_
#define _CPUINSTR_H_

#include "types.h"


/* 内联代码, 防止进入调用栈 */


/* 一个cpu指令, 关中断 */
static inline void cli(void) {
    __asm__ __volatile__("cli");
}

/* 开中断 */
static inline void sti(void) {
    __asm__ __volatile__("sti");
}

/* 一个cpu指令, 从端口号中读一个字节 */
static inline uint8_t inb(uint16_t port) {
    uint8_t rv;
    __asm__ __volatile__(
        "inb %[p],%[v]"
        :[v]"=a"(rv)
        :[p]"d"(port)
        :
    );
    return rv;
}

/* 一个cpu指令, 从端口号中读一个字 */
static inline uint16_t inw(uint16_t port) {
    uint16_t rv;
    __asm__ __volatile__(
        "in %[p],%[v]"
        :[v]"=a"(rv)
        :[p]"d"(port)
        :
    );
    return rv;
}

/* 一个cpu指令, 向端口写一个字节 */
static inline void outb(uint16_t port, uint8_t v) {
    __asm__ __volatile__(
        "outb %[_v],%[_port]"
        :
        :[_port]"d"(port), [_v]"a"(v)
        :
    );
}

static inline void lgdt(uint32_t start_addr, uint32_t size) {
    struct {
        uint16_t limit;
        uint16_t start15_0;
        uint16_t start32_16;
    }gdt;

    // 高8位
    gdt.start32_16 = start_addr >> 16;
    // 低8位
    gdt.start15_0 = start_addr & 0xFFFF;
    // gdt大小
    gdt.limit = size-1;

    __asm__ __volatile__(
        "lgdt %[g]"
        :
        :[g]"m"(gdt)
        :    
    );
}

static inline void lidt(uint32_t start_addr, uint32_t size) {
    struct {
        uint16_t limit;
        uint16_t start15_0;
        uint16_t start32_16;
    }idt;

    idt.start32_16 = start_addr >> 16;
    idt.start15_0 = start_addr & 0xFFFF;
    idt.limit = size-1;

    __asm__ __volatile__(
        "lidt %[g]"
        :
        :[g]"m"(idt)
        :    
    );
}

static inline uint8_t read_cr0(void) {
    uint32_t cr0;
    __asm__ __volatile__(
        "mov %%cr0, %[v]"
        :[v]"=r"(cr0)
        :
        :
    );
    return cr0;
}

static inline uint32_t read_eflags(void) {
    uint32_t eflags;

    __asm__ __volatile__(
        "pushf\n\t"
        "pop %%eax"
        :"=a"(eflags)
        :
        :
    );

    return eflags;
}

static inline void write_eflags(uint32_t eflags) {
    __asm__ __volatile__(
        "push %%eax\n\t"
        "popf"
        :
        :"a"(eflags)
        :
    );
}

static inline uint8_t write_cr0(uint32_t v) {
    __asm__ __volatile__(
        "mov %[v],%%cr0"
        :
        :[v]"r"(v)
        :
    );
}

static inline void far_jump(uint32_t selector, uint32_t offset) {
	uint32_t addr[] = {offset, selector };
    /* 留意此处的汇编代码, jmp用汇编16位:16位的立即数描述段地址和偏移地址
        我们应当知道.code16下只允许16位长度的立即数, 因此, 此处是否可以使用寄存器脱离这个限制? 跳到1M以上的地方？
        
    */

   
	__asm__ __volatile__(
        "ljmpl *(%[a])"
        :
        :[a]"r"(addr)
        :
    );
}

static inline void hlt() {
    __asm__ __volatile__("hlt");
}

/* 写tr寄存器 */
static inline void write_tr(uint16_t tss_sel) {
    __asm__ __volatile__(
        "ltr %%ax"
        :
        :"a"(tss_sel)
        :
    );
}


#endif


/* 内联汇编的语法

asm(
    汇编语句
    :输出操作数(可选)
    :输入操作数(可选)
    :被破环的寄存器列表(可选)
);

当汇编语言中有些数据需要储存到C语言变量中时, 使用输出操作数


https://www.yuque.com/lishutong-docs/diyx86os/al10bg
 */



// /*下面的代码将3输出到c中*/
// static void test1(void) {
//     char c;
//     asm("mov $3,%[out]":[out]"=r"(c));
// }

/* 解析上面的汇编程序 
    变量需要映射到寄存器上, 才能完成操作, r 代表random, 即随便选择一个寄存器执行这个操作
    输出操作数会在汇编执行结束之后, 寄存器写到变量上
*/


// /* 下面的程序执行 b = a 的操作 */
// static void test(void) {
//     int a = 10, b;
//     asm (   " movl %1, %%eax\n\t"
//             " movl %%eax,%0"
//             : [src]"=r"(b)
//             : [dst]"r"(a)
//     );
// }

/* 解析上面的程序
    将a映射到一个随机寄存器1上
    将寄存器1的内容写到%eax寄存器上
    选择一个寄存器2, 用%0代表
    做完这些工作之后, 将%0赋值给b
*/
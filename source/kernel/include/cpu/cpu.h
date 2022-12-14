#ifndef CPU_H
#define CPU_H

#include "os_cfg.h"
#include "comm/types.h"
#include "comm/cpuistr.h"
#include "cpu/irq.h"
// 此处循环调用了
// #include "ipc/mutex.h"

#define EFLAGS_DEFAULT  (1 << 1)
#define ELFAGS_IF       (1 << 9)


/* 设置内存对齐为1 */
#pragma pack(1)

/* 每个GDT表的内容 */
typedef struct _segment_desc_t {
    /* limit有20位, 其中有4位再attr中间 */
    uint16_t limit15_0;
    uint16_t base15_0;
    uint8_t base23_16;
    /* 这个属性其实有很多子项, 但是用一个uint16归纳
        S 是否位系统段, 置为0代表就是代码段或数据段, 如tss和系统调用问就设置为0
        DPL 权限
        P 是否存在
        SEG LIMIT(16:19) LIMIT的后4位, 总共20
        AVL
        L 与64位模式有关, 我们用不到
        D/B 指定位数, 比如这是一个32位的代码段, 运行的时候就解释为32位代码
        G 粒度是否为4K, 因为LIMIT只有20位, 因此需要大粒度来表示大空间
     */
    uint16_t attr;
    uint8_t base31_24;
} segment_desc_t;

typedef struct _tss_t {
    /* 不明东西部分2 */
    uint32_t pre_link;
    /* 栈相关 */
    uint32_t esp0, ss0;
    uint32_t esp1, ss1;
    uint32_t esp2, ss2;
    /* 虚拟内存, 现在不懂 */
    uint32_t cr3;
    /* 保存运行状态 */
    uint32_t eip, eflags;
    /* 段寄存器和通用寄存器 */
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    /* 不明东西部分1 */
    uint32_t ldt;
    uint32_t  iomap;
    /* uint32_t ssp; 待会可指定大小, 不用可以不加, 不明东西3*/
// task_state_segment
} tss_t;

#pragma pack()

/* attr参数 */

// L 位我们不置, 这与64位有关, 我们先规避
// AVL: Available and reserved bits: 这个位没有被使用, 可以由内核使用, 完全可以不用

/* 控制limit界限的单位, 值为1则单元为4KB, 见segment_desc_set的处理 */
/* 其实不用手动加这个标志位, segment_desc_set判断内存段的大小, 自动置位 */
#define SEG_G_GRANULARITY_4K    (1 << 15)
#define SEG_G_GRAUNLARITY_1B    (0 << 15)

/* 控制段的默认操作数/地址的模式(16位或者32位), 置位则32位 */
#define SEG_DB_DOUBLE_BYTE      (1 << 14)
#define SEG_DB_SIGLE_BYTE       (0 << 14)

/* 用于指示, 当前段描述符是否存在 */
#define SEG_P_PRESENT           (1 << 7)
#define SEG_P_NOPRESENT         (0 << 7)

/* 特权级相关, 我们只用0(高特权)和3(最低特权) */
#define SEG_DPL_0               (0 << 5)
#define SEG_DPL_3               (3 << 5)

/* 区分是否为系统段, NORMAL段就是code或data段 */
#define SEG_S_SYSTEM            (0 << 4)
#define SEG_S_NORMAL            (1 << 4)

/* 是否为代码段 */
#define SEG_TYPE_CODE           (1 << 3)
/* 是否为数据段 */
#define SEG_TYPE_DATA           (0 << 3)
/* 是否为任务段 */
#define SEG_TYPE_TSS            (9 << 0)

/* 表示该段是否可读或写, 对CODE段, 此为读, 一个数据寄存器可以加载code段, 然后读 对于DATA段, 此为写,
 如果ss寄存器加载了一个不能写的选择子, 那么发生CPU的GP中断*/
#define SEG_TYPE_RW             (1 << 1)


/* 关于 type:有4个位
    Accessed                            : 不懂有什么功用, 具体来说, 就是将它加载进一个寄存器的时候, 设置这个位为1
    第二位:
        Write-enable(DATA)              : 是否能写
        Read-enable(CODE)               是否允许读
    第三位                              : 对数据段和代码段不同
        1.Expansion-direction(DATA)     : 1则为expand-down, 0为expand-up
        2.Confirming(CODE)              : 不懂
    CODE/DATA                           : 0 为 code, 1为 data

关于expand-down, 此时, 基地址表示再高地址, 这样安排方便实现动态栈
关于可读代码段, 如果置位, 可以用非ss寄存器访问代码数据

一些内核中断
    如果data是expand-up的, 超过limit, 会产生GP中断
    如果data是expand-down的, 低于最低的地址, 产生stack fault
    栈数据段必须能写, 用SS加载一个不能写的data selector导致CPU的GP中断
    如果文不对题, 比如把一个code段加载给ss, 那么产生GP中断
    如果segment不存在, 那么产生NP中断

数据段的32位指示: TODO

*/

extern void gdt_init(void);
extern void gdt_reload(void);
extern void do_handler_divider(exception_frame_t *frame);
extern void do_handler_unkown(exception_frame_t * frame);
extern void install_irq(int irq_num, uint32_t pointer_as_handler);
extern int get_alloc_desc(void);
extern void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr);
extern void switch_to_tss(int tss_sel);
extern void cpu_init(void);

#endif
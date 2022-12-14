#include "cpu/cpu.h"
#include "ipc/mutex.h"

/* 保护模式下的内存管理简介

段式存储
    将内存分成多个段(内存块), 每一个GDT表对应一个段
        存储段的其实地址, 大小以及访问属性, 当前指令具体访问哪个段, 由段寄存器决定
            CS: 代码段, 指定执行代码的空间
            SS: 栈段, 指定栈的空间, 访问的时候用SS:ESP
            DS/ES/FS/GS: 数据段, 指定数据段的控制
        有多种类型的段: 数据, 代码, 栈, 门
        分段机制复杂, 采用平坦模型
            两个段, 代码段和数据段
            两个段的起始地址均位0, 限制为4GB大小, 即不管有没有那么
                大的空间, 都可访问
            

    现在没开mmu, 其实得到的就是物理地址

    段式存储, 使用逻辑地址:
        逻辑地址(选择子 + 偏移量) -> 线性地址
        1) 从寄存器取得选择子
        2) 从GDT表获得基地址
        3) 将基地址+偏移量作为物理地址, 访问对象

        如0x8:0x1234, GDT表项1中基地址为0X10000, 则线性地址为0x11234
        没开mmu, 则线性地址就是物理地址, 开启后, 物理地址进一步转化为物理地址


页式存储(启用mmu的时候才有)
    将线性地址转化位

*/



/*关于选择子
3~15位用于查询表项, 为index
第一位为RPL Requested Privilege  Level
第二位为 Table Indicator, 0为GDT, 1为LDT
*/

/* gdt表数据 */
static segment_desc_t gdt_table[GDT_TABLE_SIZE];
static rein_mutex_t mutex;

/* 设置gdt数据表的某个表项 */
void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr) {
    /* 每张gdt表8字节, 64位 */
    /* 选择子为8 代表第二张表, 索引为1 */
    /* 选择子为0 代表第一张表, 索引为0 */
    segment_desc_t *desc = gdt_table + (selector / sizeof(segment_desc_t));
    
    /* 注意这里的处理, 如果limit大于20位能表示的字节数, 那么G标志设置为1
        这样就可以表示足够大的内存段了
     */
    if (limit > 0xFFFFF) {
        attr |= SEG_G_GRANULARITY_4K;
        limit /= 0x1000;
    }

    /* 界限的一部分, 低16字节 */
    desc->limit15_0 = limit & 0xFFFF;

    /* 基地址, 分开存储, 有三块 */
    desc->base15_0 = base & 0xFFFF;
    desc->base23_16 = (base >> 16) & 0xFF;

    /*为什么选择uint32_t存limit? 因为limit有20个字节, 处理一下就行*/
    /* 属性值, 但是里面也有界限的16~19位 */
    /* 因此就12位表示的是属性 */
    /*                limit后四位内容, 将其它位置0        */
    desc->attr = attr | (((limit >> 16) & 0xF) << 8);

    /* 基地址的第三块 */
    desc->base31_24 = (base >> 24) & 0xFF;
}

/* 重新加载一个更大的gdt, 有256个表项 */
void gdt_init(void) {
    // 将其它表项都清零
    for (int i = 0; i < GDT_TABLE_SIZE; i++) {
        /* 暂时做法是, 每个选择子, 把表项中的数据全部都置为0, 其中有个p位, 此位为0时, cpu不识别此gdt项 */
        segment_desc_set(i * sizeof(segment_desc_t), 0, 0, 0);
    }

    /* 0偏移的表项保留, 处理器内部使用
        从8偏移的表项开始做
    */

    /* Base = 0 大小=4GB(0XFFFFFFFF是最后一个字节的偏移量) */
    /* 代码段和数据段, 4GB全涵盖, 可访问所有内存(大小4G)*/
    segment_desc_set(KERNEL_SELECTOR_CODE, 0, 0xFFFFFFFF, 
    /*由于G标志由第三个参数判断得到, 不指定G了, 要设置的是除了G位的其它所有位*/
        // TYPE为:EXECUTEONLY-CODE, Comforming = 0(TODO)
        SEG_P_PRESENT | SEG_DPL_0 | SEG_S_NORMAL | SEG_TYPE_CODE | SEG_DB_DOUBLE_BYTE
    );
    segment_desc_set(KERNEL_SELECTOR_DATA, 0, 0xFFFFFFFF, 
        // TYPE为: READ-WRIYTE-DATA
        SEG_P_PRESENT | SEG_DPL_0 | SEG_S_NORMAL | SEG_TYPE_DATA | SEG_TYPE_RW | SEG_DB_DOUBLE_BYTE
    );

    lgdt((uint32_t)gdt_table, sizeof(gdt_table));
}

/* 找到第一个空闲的gdt表项, 这里用锁解决竞态, attr为0则代表没被使用 */
int get_alloc_desc() {
    rein_mutex_lock(&mutex);
    for (int i = 1; i < GDT_TABLE_SIZE; i++) {
        segment_desc_t *desc = gdt_table + i;
        // 这个判断可以更换成其它判断, 只要能判断SEGMENT没有被使用即可
        if(desc->attr == 0) {
            desc->attr = SEG_P_PRESENT;
            rein_mutex_unlock(&mutex);
            return i * sizeof(segment_desc_t);
        }
    }
    
    rein_mutex_unlock(&mutex);
    return -1;
}

void switch_to_tss(int tss_sel) {
    // 切换任务的惯例, offset = 0
    far_jump(tss_sel, 0);
}


void cpu_init(void) {
    rein_mutex_init(&mutex);
    gdt_init();
    irq_init();
}
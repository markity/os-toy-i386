__asm__(".code16gcc");

#include "loader.h"

/* 保存所有可用内存块的结构体, 将来会被传入kernel中 */
boot_info_t boot_info;

/* 这个表有点问题
    1) 太小了, 只有三个gdt表项
    2) 内存区在loader这块内存, 可能被覆盖掉(注意, 设置gdt表的原理是把gdt表的地址给cpu存储)
目前暂时用这张表, 后面会重新创建一个更大的GDT表
 */
uint16_t gdt_table[][4] = {
    {   0,         0,      0,      0       },
    {   0xFFFF,    0x0000, 0x9a00, 0x00cf  },
    {   0xFFFF,    0x0000, 0x9200, 0x00cf  },
};

/* 因为bios中断只能在16位模式下使用, 因此只在此文件使用, 用于打印信息 */
static void show_msg(const char *msg) {
    char c;
    while ((c=*msg++) != '\0') {
        __asm__ __volatile__(
            "mov $0xe, %%ah\n\t"
            "mov %[input_char], %%al\n\t"
            "int $0x10"
            :
            :[input_char]"r"(c)
            :
        );
    }
}

/* 这个函数只会被一次用到, 因此尽量内联 */
static inline void detect_memory(void) {
    show_msg("Detecting memory...\r\n");

    boot_info.count = 0;

    SMAP_entry_t entry;
    uint32_t signature;
    uint8_t bytes_read;

    // 必须取0, 表示从第一块开始查
    uint32_t contID = 0;
    
    // 使用0X15中断, 0XE820服务读取内存信息
    while (1) {
        __asm__ __volatile__(
                "int  $0x15" 
				: "=a"(signature), "=c"(bytes_read), "=b"(contID)
				: "a"(0xE820), "b"(contID), "c"(24), "d"(0x534D4150), "D"(&entry)
                : 
        );

        // 一个magic number, 不要管它的作用, 它就是指示操作成功
        if(signature != 0x534D4150) {
            show_msg("failed to detect memory\r\n");
            return;
        }

        /* ACPI3.0时, 检测是否应该忽略该条目 */
        if(bytes_read > 20 && (entry.ACPI & 1) == 0) {
            /* 应当忽略掉这个条目 */
            continue;
        }

        /* Type
            1) Usable (normal) RAM          可用
            2) Reserved - unusable          保留内存, 另有作用, 不应该被使用
            3) ACPI reclaimable memory      被ACPI使用的内存
            4) ACPI NVS memory              被ACPI使用的内存
            5) Area containing bad memory   损坏的内存, BIOS自检可用查出
        */
        if(entry.Type == 1) {
            boot_info.ram_region_info[boot_info.count].start = entry.BaseL;
            boot_info.ram_region_info[boot_info.count].size = entry.LengthL;
            boot_info.count ++;
        }

        if (contID == 0 || boot_info.count == BOOT_RAM_REGION_MAX-1) {
            break;
        }
    }

    show_msg("Detecting memory OK\r\n");
}

/* 只在该文件中使用一次, 因此尽量内联 */
static inline void enter_protect_mode(void) {
    /* cli指令禁用外部中断, 此时内部异常还是能响应的, 不要把它们混为一谈 */
    cli();

    /* 开启A20地址线, 现在就可以访问更多的内存了 */
    uint8_t v = inb(0x92);
    outb(0x92, v|0b10);

    /* 加载GDT表 */
    lgdt((uint32_t)gdt_table, sizeof(gdt_table));

    /* 开启保护模式使能位CR0, 并jmp到32位代码中去 */
    uint32_t cr0 = read_cr0();
    write_cr0(cr0 | (1<<0));
    far_jump(8, (uint32_t)protect_entry);
}

void loader16_entry(void) {
    show_msg("Loading...\r\n");

    /* 最终读到两块区域, 640KB和128M两块 */
    detect_memory();

    /* 切换保护模式 */
    enter_protect_mode();
}

/* INT 0x15检测内存, BIOS中断
    第一次调用的时候, ES:DI存储保存读取的信息的存储信息
        入参:
            清除EBX, 设置为0, 这个寄存器将被后续使用
            EDX需要设置成：0x534D4150
            EAX设置成：0xE820
            ECX设置成：24
        调用: INT 0x15
        返回结果：   
            EAX = 0x534D4150
            CF标志清0
            EBX被设置成某个数值用于下次调用
            CL=实际读取的字节数
    后续调用:
        入参:
            EDX需要设置成：0x534D4150
            EAX重设为0xE820
            ECX重设为24
        调用: INT 0X15
        返回结果:
            EAX = 0x534D4150
            CF标志清0
            如果EBX=0, 则表明读取完毕, 指示没有下一条了
*/


/* 解决一个疑惑: 为什么实模式能访问32位寄存器呢
    我的直觉是实模式下是16位, 因此不能访问32位寄存器
    但是是可以的, 不要带有这种奇怪的直觉
 */

/* 之前老师写了 mov $_start, %esp, 这个$_start究竟被汇编器解释为什么呢?
        数值罢了, 它没有大小限制, 不要用位数限制它
*/
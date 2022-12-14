#ifndef _ELF_H_
#define _ELF_H_

#include "types.h"

// ELF相关数据类型
typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Sword;
typedef uint32_t Elf32_Word;

#pragma pack(1)

// ELF Header

// ELF 鉴别码, 魔数
#define ELF_MAGIC0      0x7F
#define ELF_MAGIC1      0X45
#define ELF_MAGIC2      0X4c
#define ELF_MAGIC3      0X46

// ELF 文件类型 MAGIC[4]
#define ELF_CLASS_32    0X1
#define ELF_CLASS_64    0x2

// ELF 文件编码格式 MAGIC[5]
#define ELF_DATA_LSB    0x1
#define ELF_DATA_MSB    0x2

// ELF 版本, 只能为1, MAGIC[6]
#define ELF_VERSION     0x1

// 保留的字段 MAGIC[7], MAGIC[8]....
#define ELF_PAD         0x0

// ELF TYPE
#define ET_EXEC         2       // 可执行文件
// ELF MACHINE
#define EM_386          3       // 80386处理器

typedef struct _Elf32_Ehdr {
    // 提供鉴别Elf身份的功能, 也就是一个魔数
    uint8_t e_ident[16];
    // 类型, 比如可执行文件
    Elf32_Half e_type;
    // 机器类型, 比如i386
    Elf32_Half e_machine;
    // 版本, 取1, elf是一以贯之, 再不修订的标准
    Elf32_Word e_version;

    // 下面的字段不再解释, 见elf解析方法
    Elf32_Addr e_entry;
    Elf32_Off e_phoff;
    Elf32_Off e_shoff;
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
}Elf32_Ehdr;

// PHER TYPE 表明是一个可加载类型
#define PT_LOAD         1       // 可加载类型

// program header 结构体
typedef struct _Elf32_Phdr {
    Elf32_Word p_type;
    Elf32_Off p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
} Elf32_Phdr;

#pragma pack()

#endif //OS_ELF_H

/*
知识拓展ELF四区
.text   存放代码
.data   存放数据, 全局的或静态的, 有初始化的值
.bss    未初始化的变量: 全局变量或静态变量(内容都为0的区域)
.rodata 顶层const的变量, 以及字符串常量
多源文件, 会由编译器分析合并, 最终生成的elf文件, 每个区只有一个
默认情况下其中顺序如下
.text
.rodata
.data
.bss
这解释了为什么.bss填充在最后才来
*/

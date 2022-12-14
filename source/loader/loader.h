#ifndef LOADER_H
#define LOADER_H

#include "comm/info.h"
#include "comm/cpuistr.h"
#include "comm/elf.h"

/* 汇编代码入口, 外部符号 */
extern void protect_entry(void);

// 声明外部符号
extern boot_info_t boot_info;

#endif
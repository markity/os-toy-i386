/* 定义一些常见的数据结构 */

#ifndef _TYPES_H_
#define _TYPES_H_

#include "info.h"

typedef unsigned long uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;


typedef struct _boot_info {
    struct _ram_region_info {
        uint32_t start;
        uint32_t size;
    } ram_region_info[BOOT_RAM_REGION_MAX];
    int count;
} boot_info_t;

/* 一个结构体, 存放0x15中断得到的信息 */
typedef struct _SMAP_entry {
    uint32_t BaseL;     // 基地址的低32位
    uint32_t BaseH;     // 基地址的高32位
    uint32_t LengthL;   // Length的低32位
    uint32_t LengthH;   // Length的高32位
    uint32_t Type;      // 条目类型, 值为1时表明为我们可用的RAM空间
    uint32_t ACPI;      // bit0=1时表明此条目应当被忽略
} __attribute__((packed)) SMAP_entry_t;

#endif
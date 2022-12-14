/* 这个文件记录一些基本的参数 */


#ifndef _INFO_H_
#define _INFO_H_

/* BOOT 程序的地址, 这是绝对地址 */
#define BOOT_ADDR   0x7c00
/* BOOT 程序的最大大小 */
#define BOOT_MAX_SIZE   512

/* LOADER 程序的地址, 这是绝对地址, 0x8000~0x10000刚好512*64个字节 */
#define LOADER_ADDR 0x8000
/* LOADER 程序的最大大小 */
#define LOADER_MAX_SIZE (512*64)

/* os最多能识别的内存块 */
#define BOOT_RAM_REGION_MAX 10

/* 磁盘单个扇区512个字节 */
#define SECTOR_SIZE 512

/* 磁盘kernel代码开始的扇区是第几个 */
/* 其中0扇区存BOOT程序, 1~64扇区存LOADER程序, 70以及以后最大500个扇区存系统代码 */
#define KERNEL_BEGIN_SECTOR 65

/* kernel代码最多占用500个扇区 */
#define KERNEL_MAX_SECTORS_IN_SIZE 500

/* ELF内核文件在这里, 先将ELF内核文件放在1M处, 之后再加载到0x10000(64k)处 */
#define SYS_KERNEL_LOAD_ADDR (void*)(1024 * 1024)
/* kernel代码本体在这里 */
#define SYS_KERNEL_CODE_ADDR (void*)(64*1024)


#endif
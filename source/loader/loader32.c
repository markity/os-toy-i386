#include "loader.h"

/* LBA48 此处用到外部设备来读磁盘, 只是使用一次, 因此尽量声明为inline */
static inline void read_disk(uint32_t sector, uint32_t sector_count, void *buf) {
    // 选择LAB48模式, 选择主盘
    outb(0x1F6, 0xE0);

    outb(0x1F2, (uint8_t)(sector_count >> 8));  // 扇区数的高字节
    outb(0x1F3, (uint8_t)(sector >> 24));       // LBA4
    outb(0x1F4, 0);                             // LBA5
    outb(0x1F5,0);                              // LBA6
    
    outb(0x1F2, (uint8_t)sector_count);         // 扇区数的低字节
    outb(0x1F3, (uint8_t)sector);               // LBA1
    outb(0x1F4, (uint8_t)(sector >> 8));        // LBA2
    outb(0x1F5, (uint8_t)(sector >> 16));       // LBA3

    // 0x1F7即为命令端口, 又为状态端口
    // 发送READ SECTORS EXT命令
    outb(0x1F7, 0x24);

    uint16_t *databuf = (uint16_t *)buf;
    while (sector_count --) {
        while((inb(0x1F7) & 0x88) != 0x8) {}

        for (int i = 0; i < SECTOR_SIZE / 2; i++) {
            *databuf++ = inw(0x1F0);
        }
    }
}


/* 只在该文件内使用了一次, 因此应该尽量声明为内联函数 */
static inline uint32_t reload_elf_file(void * file_buffer) {
    Elf32_Ehdr *elf_hdr = (Elf32_Ehdr *)file_buffer;

    // 鉴别ELF身份
    if(elf_hdr->e_ident[0] != ELF_MAGIC0 || elf_hdr->e_ident[1] != ELF_MAGIC1 || elf_hdr->e_ident[2] != ELF_MAGIC2 ||
        elf_hdr->e_ident[3] != ELF_MAGIC3) {
        return 0;
    }

    Elf32_Phdr *phdr_start = file_buffer + elf_hdr->e_phoff;
    for(int i = 0; i < elf_hdr->e_phnum; i++) {
        // program header是连续的, 在file_buffer + elf_hdr->e_phoff上为第一个
        Elf32_Phdr *phdr = phdr_start + i;
        // program header的类型, 是否为可加载类型
        if(phdr->p_type != PT_LOAD) {
            continue;
        }

        uint8_t *src = file_buffer + phdr->p_offset;
        uint8_t *dest = (uint8_t*)(phdr->p_paddr);

        // 拷贝段的数据
        for(int j = 0; j < phdr->p_filesz;j++) {
            *(dest++) = *(src++);
        }

        // 因为.bss在最后, 此时可以手动填充0, 体会一下, 如果.bss在前面, 填充的数将为0, 这样就不利于节省空间
        // 因此要节省空间, 必须将.bss放在最后
        uint32_t fill_bytes = phdr->p_memsz - phdr->p_filesz;
        for (int j = 0; j < fill_bytes;j++) {
            *dest++ = 0;
        }
    }
    return elf_hdr->e_entry;
}


/* 现在是32位了, 读取磁盘不能用BIOS中断了 */
void load_kernel(void) {
    /* 将elf文件读到1M处, 我们可以看到boot_info得到第二块内存的起点便是1024*1024 */
    read_disk(KERNEL_BEGIN_SECTOR, KERNEL_MAX_SECTORS_IN_SIZE, SYS_KERNEL_LOAD_ADDR);
    uint32_t kernel_entry = reload_elf_file((void *)SYS_KERNEL_LOAD_ADDR);
    if(kernel_entry == 0) {
        /* 一个致命的问题, 进行死机 */
        while (1);
    }

    /* 跳转到内核中执行 */
    ((void(*)(boot_info_t*))kernel_entry)(&boot_info);
}
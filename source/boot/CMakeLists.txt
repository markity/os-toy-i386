
project(boot LANGUAGES ASM)

# 使用自定义的链接器
set(CMAKE_ASM_LINK_EXECUTABLE "x86_64-elf-ld <OBJECTS> -m elf_i386  -Ttext=0x7c00 -o ${PROJECT_BINARY_DIR}/boot.elf")

# 将所有的汇编、C文件加入工程
# 注意保证start.asm在最前头，这样对应的程序会放在bin文件开头，从而在加载到0x7c00时能在开头处
# 扫描boot目录下所有.h和.c的文件
add_executable(boot start.S)

# bin文件生成，写入到image目录下
add_custom_command(TARGET boot
                   POST_BUILD
                   COMMAND x86_64-elf-objcopy -O binary boot.elf ${CMAKE_SOURCE_DIR}/img/boot.bin
                   COMMAND x86_64-elf-objdump -x -d -S -m i8086 ${PROJECT_BINARY_DIR}/boot.elf > boot_dis.txt
                   COMMAND x86_64-elf-readelf -a ${PROJECT_BINARY_DIR}/boot.elf > boot_elf.txt
)

project(kernel LANGUAGES C)

# 使用自定义的链接器
set(CMAKE_C_LINK_EXECUTABLE "x86_64-elf-ld <OBJECTS> -m elf_i386  -T ${PROJECT_SOURCE_DIR}/kernel.lds -o ${PROJECT_BINARY_DIR}/kernel.elf")

# 将所有的汇编、C文件加入工程
# 注意保证start.asm在最前头，这样对应的程序会放在bin文件开头，从而在加载到0x7c00时能在开头处
file(GLOB_RECURSE C_LIST "*.c" "*.h")
add_executable(kernel init/start.S ${C_LIST})

# bin文件生成，写入到image目录下
add_custom_command(TARGET kernel
                   POST_BUILD
                   # 此处-S参数可清除一些debug相关的内容
                   COMMAND x86_64-elf-objcopy -S kernel.elf ${CMAKE_SOURCE_DIR}/img/kernel.elf
                   COMMAND x86_64-elf-objdump -x -d -S -m i386 ${PROJECT_BINARY_DIR}/kernel.elf > kernel_dis.txt
                   COMMAND x86_64-elf-readelf -a ${PROJECT_BINARY_DIR}/kernel.elf > kernel_elf.txt
)
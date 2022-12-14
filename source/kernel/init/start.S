#include "os_cfg.h"

/* 现在是保护模式, 默认生成32位指令 */
.text
.global _start
.extern kernel_init
.extern init_main

_start:

    /* 进行一个传参准备, 配合C语言函数, 这里不符合c convention, 因为马上要切换栈, 完全进入C代码 */
    mov 0x4(%esp), %eax
    push %eax
    
    call kernel_init

    /* 设置了新的栈, 的内容在内核中 */
    mov $(stack + KERNEL_STACK_SIZE), %esp
    

    jmp init_main

.bss
.comm stack, KERNEL_STACK_SIZE



.macro exception_handler name num with_err_code

    /  * 缺省的中断处理函数 */
.text
.global exception_handler_\name
.extern do_handler_\name
exception_handler_\name:
/* 异常或中断进入的时候, 必须保护上下文
    会自动压入一些寄存器, eflags, cs, eip, error code
    iret会弹出并恢复eflags, cs, eip, 清除栈中的error code
    我们需要保护eax, ebx, ecx, edx, esi, edi, esp, ebp
 */
    /* 中断发生时, 硬件压入
        push eflags
        push cs
        push eip
        push error code
    */


    /* 如果一个异常时没有err_code的, 我们需要压入一个值
        用于适配结构体
     */
    .if \with_err_code == 0
        push $0
    .endif

    push $\num
    pusha
    /*
    这条指令等价于, 压入所有通用寄存器
    temp := esp
    push eax
    push ecx
    push edx
    push ebx
    push temp
    push ebp
    push esi
    push edi
    将通用寄存器全部压入栈中
    */


    push %ds
    push %es
    push %fs
    push %gs

    /* 相当于一个c语言传参
    do_handler_unkown
    (gs, fs, es, ds, edi, esi, ebp, esp, ebx, edx,
     ecx, eax, err, eip, cs, eflags)
    */
    /* 这里我们简化参数, 用结构体指针简化问题, 妙啊! */
    push %esp
    call do_handler_\name
    /* 一个出栈的过程 */
    add $4, %esp
    
    pop %gs
    pop %fs
    pop %es
    pop %ds

    popa

    /* 弹栈操作, 我们需要手动弹出num和errorcode, iret会要求你先将errorcode手动弹出 */
    add $8, %esp

    iret

.endm

/* 用宏重用代码 */

exception_handler unknown,                      -1, 0
exception_handler divider,                      0, 0
exception_handler Debug,                        1, 0
exception_handler NMI,                          2, 0
exception_handler breakpoint,                   3, 0
exception_handler overflow,                     4, 0
exception_handler bound_range,                  5, 0
exception_handler invalid_opcode,               6, 0
exception_handler device_unavailable,           7, 0
exception_handler coprocessor_segment_overrun,  9, 0
exception_handler double_fault,                 8, 1
exception_handler invalid_tss,                  10, 1
exception_handler segment_not_present,          11, 1
exception_handler stack_segment_fault,          12, 1
exception_handler general_protection,           13, 1
exception_handler page_fault,                   14, 1
exception_handler fpu_error,                    16, 0
exception_handler alignment_check,              17, 1
exception_handler machine_check,                18, 0
exception_handler smd_exception,                19, 0
exception_handler virtual_exception,            20, 0
exception_handler control_protection_exception  21, 1
exception_handler timer,                        0x20, 0 

.text
.global simple_switch
/* simple_switch(&from, to): 传递from, to 两个参数 */
simple_switch:
    /* from任务的栈顶指针 */
    mov 4(%esp), %eax

    /* to任务的栈顶指针 */
    mov 8(%esp), %edx

    /* 根据汇编abi convention保存上下文 */
    push %ebp
    push %ebx
    push %esi
    push %edi
    mov %esp, (%eax)


    /* 恢复下文 */
    mov %edx, %esp
    pop %edi
    pop %esi
    pop %ebx
    pop %ebp

    /* 进入新任务, 已经在task.c中设置 */
    ret
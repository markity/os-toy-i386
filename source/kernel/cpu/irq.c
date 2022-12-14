#include "cpu/irq.h"

/* 最多支持256个表项, 我们只使用128个 */
static gate_desc_t idt_table[IDT_TABLE_NR];

static void dump_core_regs(exception_frame_t *frame) {
    log_printfln("IRQ: %d, error code: %d", frame->num, frame->error_code);
    log_printfln(
        "CS: %d\r\n"
        "DS: %d\r\n"
        "ES: %d\r\n"
        "SS: %d\r\n"
        "FS: %d\r\n"
        "GS: %d\r",
        /* 稍后init完毕后, 开启中断之前四个段寄存器是相同的, 因此用ES代替SS*/
        frame->cs, frame->ds, frame->es, frame->ds, frame->fs, frame->gs
    );
    log_printfln(
        "EAX: 0x%x\r\n"
        "EBX: 0x%x\r\n"
        "ECX: 0x%x\r\n"
        "EDX: 0x%x\r\n"
        "EDI: 0x%x\r\n"
        "ESI: 0x%x\r\n"
        "EBP: 0x%x\r\n"
        "ESP: 0x%x\r",
        frame->eax, frame->ebx, frame->ecx, frame->edx, frame->edi,
        frame->esi, frame->ebp, frame->esp
    );
    log_printfln(
        "EIP: 0x%x\r\n"
        "EFLAGS: 0x%x",
        frame->eip,frame->eflags
    );
}

void do_default_handler(exception_frame_t *frame,const char* msg) {
    /* 打印寄存器信息 */
    log_println("-------------");
    log_printfln("IRQ/EXCEPTION happened: %s", msg);
    dump_core_regs(frame);
    log_println("-------------");

    /* 做死机处理 */
    for(;;) {
        hlt();
    }
}

void do_handler_unknown(exception_frame_t *frame) {
    do_default_handler(frame,"unknown exception");
}

void do_handler_divider(exception_frame_t *frame) {
    do_default_handler(frame, "divider exception");
}

void do_handler_Debug(exception_frame_t *frame) {
    do_default_handler(frame, "Debug exception");
}

void do_handler_NMI(exception_frame_t *frame) {
    do_default_handler(frame, "NMI exception");
}

void do_handler_breakpoint(exception_frame_t *frame) {
    do_default_handler(frame, "breakpoint exception");
}

void do_handler_overflow(exception_frame_t *frame) {
    do_default_handler(frame, "overflow exception");
}

void do_handler_bound_range(exception_frame_t *frame) {
    do_default_handler(frame, "bound_range exception");
}

void do_handler_invalid_opcode(exception_frame_t *frame) {
    do_default_handler(frame, "invalid_opcode exception");
}

void do_handler_device_unavailable(exception_frame_t *frame) {
    do_default_handler(frame, "device_unavailable exception");
}

void do_handler_double_fault(exception_frame_t *frame) {
    do_default_handler(frame, "double_fault exception");
}

void do_handler_coprocessor_segment_overrun(exception_frame_t *frame) {
    do_default_handler(frame, "coprocessor_segment_overrun exception");
}

void do_handler_invalid_tss(exception_frame_t *frame) {
    do_default_handler(frame, "invalid_tss exception");
}

void do_handler_segment_not_present(exception_frame_t *frame) {
    do_default_handler(frame, "segment_not_present exception");
}

void do_handler_stack_segment_fault(exception_frame_t *frame) {
    do_default_handler(frame, "stack_segment_fault exception");
}

void do_handler_general_protection(exception_frame_t *frame) {
    do_default_handler(frame, "general_protection exception");
}

void do_handler_page_fault(exception_frame_t *frame) {
    do_default_handler(frame, "page_fault exception");
}

void do_handler_fpu_error(exception_frame_t *frame) {
    do_default_handler(frame, "fpu_fault exception");
}

void do_handler_alignment_check(exception_frame_t *frame) {
    do_default_handler(frame, "alignment_check exception");
}

void do_handler_machine_check(exception_frame_t *frame) {
    do_default_handler(frame, "machine_check exception");
}

void do_handler_smd_exception(exception_frame_t *frame) {
    do_default_handler(frame, "smd_exception exception");
}

void do_handler_virtual_exception(exception_frame_t *frame) {
    do_default_handler(frame, "virtual_exception exception");
}

void do_handler_control_protection_exception(exception_frame_t *frame) {
    do_default_handler(frame, "control_protection_exception exception");
}

static void gate_desc_set(gate_desc_t * desc, uint16_t selector, uint32_t offset, uint16_t attr) {
    desc->offset15_0 = offset & 0xFFFF;
    desc->selector = selector;
    desc->attr = attr;
    desc->offset31_16 = (offset >> 16) & 0xFFFF;
}

// 向8259发送END OF INTERRUPT信号, 此时8259着手下一个信号发送
void pic_send_eoi(int irq_num) {
    irq_num -= IRQ_PIC_START;

    // 从片也可能需要发送EOI
    if (irq_num >= 8) {
        outb(PIC1_OCW2, PIC_OCW2_EOI);
    }

    outb(PIC0_OCW2, PIC_OCW2_EOI);
}

void irq_install(int irq_num, uint32_t pointer_as_handler) {
    gate_desc_set( idt_table+irq_num, KERNEL_SELECTOR_CODE, (uint32_t)pointer_as_handler,
        GATE_P_PRESENT | GATE_DPL0 | GATE_TYPE_INT
    );
}

void irq_enable(int irq_num) {
    /* 只有两块8259芯片 */
    if (irq_num < IRQ_PIC_START || irq_num > IRQ_PIC_START + 15) {
        return;
    }

    irq_num -= IRQ_PIC_START;

    /* 对应第一块 */
    if (irq_num < 8) {
        uint8_t mask = inb(PIC0_IMR) & ~(1<<irq_num);
        outb(PIC0_IMR, mask);
    } else {
        irq_num -= 8;
        uint8_t mask = inb(PIC1_IMR) & ~(1<<irq_num);
        outb(PIC1_IMR, mask);
    }
}

void irq_disable(int irq_num) {
        /* 只有两块8259芯片 */
    if (irq_num < IRQ_PIC_START || irq_num > IRQ_PIC_START + 15) {
        return;
    }

    irq_num -= IRQ_PIC_START;

    /* 对应第一块 */
    if (irq_num < 8) {
        uint8_t mask = inb(PIC0_IMR) | (1<<irq_num);
        outb(PIC0_IMR, mask);
    } else {
        irq_num -= 8;
        uint8_t mask = inb(PIC1_IMR) | (1<<irq_num);
        outb(PIC1_IMR, mask);
    }
}

/* cli和sti决定cpu是否忽略来自8259的中断请求, 即不发送中断应答, 此时, 如果8259发来中断请求, cpu不应答, 但会保存(不丢弃)这个中断请求 */
void irq_disable_global (void) {
    cli();
}

void irq_enable_global(void) {
    sti();
}

/*简述8259工作方式
    外部发送一个中断信号, 到达IMR(中断屏蔽寄存器)处, 如果被屏蔽, 这个中断信号则被丢弃, 可通过设置IMR寄存器来控制是否丢弃传送的信号
    接下来信号被送到IRR(中断暂存寄存器)处. 此时由于存在待处理的中断信号, 8259将打开INTR, 尝试通知CPU处理中断请求, 当CPU有空的时候
    它会检查INTR(如果没有进行cli指令), 然后着手处理中断. 具体来说, 就是向INTA(中断应答)发送信号, 然后8259判优找到优先级最高的中断
    , 将中断请求放在ISR(中断服务寄存器)里面, 然后将对应IRR置位0, 接下来将中断编号写入IVR的低3位(ICW2写入的, 它的低三位被利用了),
    CPU还会送来第二个INTA信号, 当收到此信号后, 芯片将IVR中的内容, 也就是此中断的中断号送上通向CPU的数据线
*/

static void pic_init(void) {
    // 边缘触发，级联，需要配置icw4, 8086模式
    // ICW1是初始化8259的第一条命令, 向它发送信号意味着配置芯片的开始, 对8086的cpu, 设置为0x11
    outb(PIC0_ICW1, 0x11);
    outb(PIC1_ICW1, 0x11);

    // 对应的中断号起始序号0x20
    outb(PIC0_ICW2, IRQ_PIC_START);
    outb(PIC1_ICW2, IRQ_PIC_START + 8);

    // 主片IRQ2有从片, 告知从片被接在什么地方, 即IRQ2(前面有IRQ0, IRQ1)
    outb(PIC0_ICW3, 1 << 2);
    outb(PIC1_ICW3, 2);

    outb(PIC0_ICW4, 0x01);
    outb(PIC1_ICW4, 0x01);


    // 禁止所有中断, 允许从PIC1传来的中断
    outb(PIC0_IMR, 0xFF & ~(1 << 2));

    // 禁用PIC1的所有中断
    outb(PIC1_IMR, 0xFF);
}

void irq_init(void) {
    /* 首先把所有中断门做缺省的处理程序 */
    for (int i = 0; i < IDT_TABLE_NR; i++) {
        gate_desc_set(idt_table+i, KERNEL_SELECTOR_CODE, (uint32_t)(&exception_handler_unknown),
        GATE_P_PRESENT | GATE_DPL0 | GATE_TYPE_INT
        );
    }

    irq_install(IRQ0_DE, (uint32_t)(&exception_handler_divider));
    irq_install(IRQ1_DB, (uint32_t)(&exception_handler_Debug));
    irq_install(IRQ2_NMI, (uint32_t)(&exception_handler_NMI));
    irq_install(IRQ3_BP, (uint32_t)(&exception_handler_breakpoint));
    irq_install(IRQ4_OF, (uint32_t)(&exception_handler_overflow));
    irq_install(IRQ5_BR, (uint32_t)(&exception_handler_bound_range));
    irq_install(IRQ6_UD, (uint32_t)(&exception_handler_invalid_opcode));
    irq_install(IRQ7_NM, (uint32_t)(&exception_handler_device_unavailable));
    irq_install(IRQ8_DF, (uint32_t)(&exception_handler_double_fault));
    irq_install(IRQ9_CSO, (uint32_t)(&exception_handler_coprocessor_segment_overrun));
    irq_install(IRQ10_TS, (uint32_t)(&exception_handler_invalid_tss));
    irq_install(IRQ11_NP, (uint32_t)(&exception_handler_segment_not_present));
    irq_install(IRQ12_SS, (uint32_t)(&exception_handler_stack_segment_fault));
    irq_install(IRQ13_GP, (uint32_t)(&exception_handler_general_protection));
    irq_install(IRQ14_PF, (uint32_t)(&exception_handler_page_fault));
    irq_install(IRQ16_MF, (uint32_t)(&exception_handler_fpu_error));
    irq_install(IRQ17_AC, (uint32_t)(&exception_handler_alignment_check));
    irq_install(IRQ18_MC, (uint32_t)(&exception_handler_machine_check));
    irq_install(IRQ19_XM, (uint32_t)(&exception_handler_smd_exception));
    irq_install(IRQ20_VE, (uint32_t)(&exception_handler_virtual_exception));
    irq_install(IRQ21_CPE, (uint32_t)(&exception_handler_control_protection_exception));


    lidt((uint32_t)idt_table, sizeof(idt_table));

    /* 初始化两块外部设备中断芯片 */
    pic_init();
}

irq_state_t irq_enter_protection(void) {
    irq_state_t state = read_eflags();
    irq_disable_global();
    return state;
}

// 根据调用约定, eflags的值不管是啥
extern void irq_leave_protection(irq_state_t state) {
    write_eflags(state);
}
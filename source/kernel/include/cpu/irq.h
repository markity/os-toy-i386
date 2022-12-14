/**
 * 中断处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef IRQ_H
#define IRQ_H

#include "comm/cpuistr.h"
#include "os_cfg.h"
#include "tools/log.h"

// 中断号码
#define IRQ0_DE             0
#define IRQ1_DB             1
#define IRQ2_NMI            2
#define IRQ3_BP             3
#define IRQ4_OF             4
#define IRQ5_BR             5
#define IRQ6_UD             6
#define IRQ7_NM             7
#define IRQ8_DF             8
#define IRQ9_CSO            9
#define IRQ10_TS            10
#define IRQ11_NP            11
#define IRQ12_SS            12
#define IRQ13_GP            13
#define IRQ14_PF            14
// 15号保留
#define IRQ16_MF            16
#define IRQ17_AC            17
#define IRQ18_MC            18
#define IRQ19_XM            19
#define IRQ20_VE            20
#define IRQ21_CPE           21

#define IRQ0_TIMER          0x20

/* 中断向量表支持最大256项, 只使用到128项*/
#define IDT_TABLE_NR 128


#pragma pack(1)


/* 门表项 */
typedef struct _int_gate_desc_t {
    uint16_t offset15_0;
    uint16_t selector;
    uint16_t attr;
    uint16_t offset31_16;
} gate_desc_t;


/**
 * 中断发生时相应的栈结构，暂时为无特权级发生的情况
 */
typedef struct _exception_frame_t {
    // 结合压栈的过程，以及pusha指令的实际压入过程
    int gs, fs, es, ds;
    int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    int num;
    int error_code;
    int eip, cs, eflags;
}exception_frame_t;

#pragma pack()

extern void irq_init(void);
extern void irq_install(int irq_num, uint32_t handler);

extern void exception_handler_unknown(void);
extern void exception_handler_divider(void);
extern void exception_handler_Debug(void);
extern void exception_handler_NMI(void);
extern void exception_handler_breakpoint(void);
extern void exception_handler_overflow(void);
extern void exception_handler_bound_range(void);
extern void exception_handler_invalid_opcode(void);
extern void exception_handler_device_unavailable(void);
extern void exception_handler_double_fault(void);
extern void exception_handler_coprocessor_segment_overrun(void);
extern void exception_handler_invalid_tss(void);
extern void exception_handler_segment_not_present(void);
extern void exception_handler_stack_segment_fault(void);
extern void exception_handler_general_protection(void);
extern void exception_handler_page_fault(void);
extern void exception_handler_fpu_error(void);
extern void exception_handler_alignment_check(void);
extern void exception_handler_machine_check(void);
extern void exception_handler_smd_exception(void);
extern void exception_handler_virtual_exception(void);
extern void exception_handler_control_protection_exception(void);


/* 门的类型: 包含门类型为中断门, 32位等配置, 直接用一个参数归纳 */
#define GATE_TYPE_INT       (0xE << 8)
/* 中断门是否存在 */
#define GATE_P_PRESENT      (1 << 15)
/* 权限相关, 其实只用到DPL0 */
#define GATE_DPL0           (0 << 13)
#define GATE_DPL3           (3 << 13)



// PIC控制器相关的寄存器及位配置
#define PIC0_ICW1			0x20
#define PIC0_ICW2			0x21
#define PIC0_ICW3			0x21
#define PIC0_ICW4			0x21
#define PIC0_OCW2			0x20
#define PIC0_IMR			0x21

#define PIC1_ICW1			0xa0
#define PIC1_ICW2			0xa1
#define PIC1_ICW3			0xa1
#define PIC1_ICW4			0xa1
#define PIC1_OCW2			0xa0
#define PIC1_IMR			0xa1

#define PIC_OCW2_EOI		(1 << 5)		// 1 - 非特殊结束中断EOI命令

#define IRQ_PIC_START		0x20			// PIC中断起始号

extern void irq_enable(int irq_num);
extern void irq_disable(int irq_num);
extern void irq_disable_global(void);
extern void irq_enable_global(void);

extern void pic_send_eoi(int irq);

typedef uint32_t irq_state_t;
extern irq_state_t irq_enter_protection(void);
extern void irq_leave_protection(irq_state_t state);


#endif

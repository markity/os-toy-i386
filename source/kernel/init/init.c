#include "init.h"
#include "tools/list.h"

/* 需要把内存容量传递给内核 */
void kernel_init(boot_info_t *meminfo) {
    // 加载GDT表, 初始化定时器, 初始化任务管理器, 初始化串口
    // 紧接着跳入init_main执行代码
    // 此时task_manager里面的curr_task为(task_t *)0, 需要保证初始化init任务前, 包含互斥锁操作的正确性

    /* 初始化所有GDT表
        0 号系统保留
        1 号代码段
        2 号数据段
        此处初始化GDT表, 没有改变CS和其它段寄存器的值, 因为之前建立的临时表也是这些数据
        这里多做的工作是初始化更多的表, 以供后面更多TSS段用
     */
    cpu_init();

    // 随便入一个不存在的内中断, 发生CPU的GP异常
    // __asm__ __volatile__("int $0x80");

    /* 开启计时器 */
    timer_init();

    task_manager_init();

    log_init();
}

/* 异常中断简介
    异常: 由于CPU内部时间引起中断, 如程序出错(非法指令, 地址越界, 除0异常)引起
        通常是由于执行了现行指令所引起的
    

    中断: 由于外部设备时间引起的中断, 如通常的磁盘中断, 打印机中断
        通常与现行指令无关, 由外部事件引起


    当异常和中断发生时, CPU暂停当时正在执行的指令, 转而去执行响应的事件处理程序

    共255种, 0~31号被内部保留, 如0是除法异常(除0)
*/


typedef struct _list_test_t {
    int i;
    list_node_t node;
} list_test_t;


// void list_test() {
//     list_test_t v;
//     v.i = 100;
//     list_test_t *v_ptr = CONTAINER_OF(&v.node, list_test_t, node);
    

//     log_printfln("%d", v_ptr->i);

//     list_t list;
//     list_init(&list);
//     list_node_t nodes[5];

//     log_printfln("list: first=0x%x, last=0x%x, count=%d", 
//         list_first(&list), list_last(&list), list_count(&list));

//     for (int i = 0; i < 5; i++) {
//         list_node_t *node = nodes+i;
//         list_node_init(node);
        
//         log_printfln("insert first to list: %d, 0x%x", i, node);
//         list_insert_last(&list, node);
//     }
//     log_printfln("list: first=0x%x, last=0x%x, count=%d", 
//         list_first(&list), list_last(&list), list_count(&list));

//     list_init(&list);
//         for (int i = 0; i < 5; i++) {
//         list_node_t *node = nodes+i;
//         list_node_init(node);
        
//         log_printfln("insert first to list: %d, 0x%x", i, node);
//         list_insert_first(&list, node);
//     }
//     log_printfln("list: first=0x%x, last=0x%x, count=%d", 
//         list_first(&list), list_last(&list), list_count(&list));

//     for (int i = 0; i < 5; i++) {
//         list_node_t *node = list_pop_first(&list);
//         log_printfln("pop first from list: %d, 0x%x", i, node);
//     }
//     log_printfln("list: first=0x%x, last=0x%x, count=%d", 
//         list_first(&list), list_last(&list), list_count(&list));

//     list_init(&list);
//         for (int i = 0; i < 5; i++) {
//         list_node_t *node = nodes+i;
//         list_node_init(node);
        
//         log_printfln("insert first to list: %d, 0x%x", i, node);
//         list_insert_first(&list, node);
//     }
//     log_printfln("list: first=0x%x, last=0x%x, count=%d", 
//         list_first(&list), list_last(&list), list_count(&list));
//     for (int i = 0; i < 5; i++) {
//         list_node_t *node = nodes+i;
//         list_pop(&list, node);
//         log_printfln("pop from list: %d, 0x%x", i, node);
//     }
//     log_printfln("list: first=0x%x, last=0x%x, count=%d", 
//         list_first(&list), list_last(&list), list_count(&list));

//     list_init(&list);
//     for (int i = 0; i < 5; i++) {
//         list_node_t *node = nodes+i;
//         list_node_init(node);
        
//         log_printfln("insert first to list: %d, 0x%x", i, node);
//         list_insert_first(&list, node);
//     }
//     log_printfln("list: first=0x%x, last=0x%x, count=%d", 
//         list_first(&list), list_last(&list), list_count(&list));
//     for (int i = 0; i < 5; i++) {
//         list_node_t *node = nodes+i;
//         list_pop_last(&list);
//         log_printfln("pop from list: %d, 0x%x", i, node);
//     }
//     log_printfln("list: first=0x%x, last=0x%x, count=%d", 
//         list_first(&list), list_last(&list), list_count(&list));
// }

/* 新建任务 */
static task_t new_task;

/* 为new_task准备一个栈, 4K空间 */
static uint32_t new_task_stack[1024];

static sem_t notify_task_print;

/* 新建任务入口 */
void new_task_entry(void) {
    int count = 0;
    for(;;) {
        sem_wait(&notify_task_print);
        log_printfln("recv notify: %d", count++);
        // sys_sched_yield();
    }
}

/* 新建任务 */
static task_t new_task2;

/* 为new_task准备一个栈, 4K空间 */
static uint32_t new_task2_stack[1024];

void new_task2_entry(void) {
    int count = 0;
    for(;;) {
        log_printfln("new task2: %d", count++);
        sys_sleep(3000);
        // sys_sched_yield();
    }
}


/* 新建任务 */
static task_t new_task3;

/* 为new_task准备一个栈, 4K空间 */
static uint32_t new_task3_stack[1024];

void new_task3_entry(void) {
    int count = 0;
    for(;;) {
        log_printfln("new task3: %d", count++);
        sys_sleep(3000);
        // sys_sched_yield();
    }
}


void init_main(void) {    
    // list_test();

    // irq_enable_global();

    /* 测试百分号 */
    // log_printfln("Kernel is running %%0");

    // /* 测试字符串 */
    // log_printfln("Kernel version %s", OS_VERSION);

    // /* 测试更多类型类型 */
    // log_printfln("Hello %d 123", 43);
    // log_printfln("Hello %d 123", -12);
    // log_printfln("%d", -12);
    // log_printfln("Hello %d %x %c 123", -12, 3000, 'C');

    // int a = 3 / 0;

    task_main_init();
    sem_init(&notify_task_print, 0);

    irq_enable_global();

    task_init(&new_task, "my new task", (uint32_t)new_task_entry, (uint32_t)(&new_task_stack[1024]));
    // task_init(&new_task2, "my new task2", (uint32_t)new_task2_entry, (uint32_t)(&new_task2_stack[1024]));
    // task_init(&new_task3, "my new task3", (uint32_t)new_task3_entry, (uint32_t)(&new_task3_stack[1024]));


    int count = 0;
    for(;;) {
        log_printfln("sending notify: %d", count++);
        // 每隔1s发一次信号, 告知task执行一定的动作
        sem_notify(&notify_task_print);
        sys_sleep(1000);
        // sys_sched_yield();
    }
}
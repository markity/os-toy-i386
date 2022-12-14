#include "tools/klib.h"

void kernel_strcpy(char *dest, const char *src) {
    while (*src)
        *dest++ = *src++;
    *dest = '\0';
}

/* 1 相等, 0 不相等 */
int kernel_str_equal(const char *s1, const char *s2) {
    if (kernel_strlen(s1) != kernel_strlen(s2)) {
        return 0;
    }
    for (uint32_t i = 0; i < kernel_strlen(s1); i++) {
        if(*s1++ != *s2++) {
            return 0;
        }
    }
    
    return 1;
}

uint32_t kernel_strlen(const char *s) {
    uint32_t count = 0;
    while (s[count] != '\0') {
        count ++;
    }
    return count;
}

void kernel_memcpy(void *dest, const void *src, uint32_t size) {
    if(size == 0)
        return;
    const uint8_t *src_ = src;
    uint8_t *dest_ = dest;
    while(size--)
        *dest_++ = *src_++;
}

int kernel_mem_equal(const void *d1, const void *d2, uint32_t size) {
    const uint8_t *p1 = d1, *p2 = d2;
    for (uint32_t i = 0; i < size; i++) {
        if(*p1++ != *p2++)
            return 0;
    }
    return 1;
}

void kernel_memset(void *dest, uint8_t c, uint32_t size) {
    uint8_t *dest_ = dest;
    while (size --)
        *dest_ = c;
}

static const char *num2ch = {"0123456789ABCDEF"};

char *kernel_itoa(char *buf, int num, int base) {
    char *p = buf;

    if (base != 2 && base != 10 && base != 16) {
        log_println("kernel_itoa: base is a invalid value");
        for(;;){}
    }

    if (num < 0 && base == 16) {
        log_println("kernel_itoa: base16 when num < 0");
        for(;;){}
    }

    int flag = 0;
    if(num < 0 && base == 10) {
        flag = 1;
        *p++ = '-';
    }

    do {
        *p++ = num2ch[ (num > 0) ? (num % base) : (-(num % base))];
        num /= base;
    }while(num);
    
    /* 颠倒顺序, 双指针算法 */
    char *start = buf + flag;
    char *end = p - 1;
    while (start < end) {
        char ch = *start;
        *start = *end;
        *end = ch;
        start ++, end--;
    }

    return p;
}

void kernel_sprintf(char *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    kernel_vsprintf(buf, fmt, args);
    va_end(args);
}


void kernel_vsprintf(char *buf, const char *fmt, va_list args) {
    /* 状态机处理 */
    enum {NORMAL, READ_FMT} state = NORMAL;

    char *current = buf;
    char ch;
    while ((ch = *fmt++)) {

        switch (state)
        {
        case NORMAL:
            /* 读到%, 更改状态机状态 */
            if(ch == '%') {
                state = READ_FMT;
            } else {
                *current++ = ch;
            }
            break;
        case READ_FMT:
            /* 字符串支持 */
            if (ch == 's') {
                const char* str = va_arg(args, const char *);
                uint32_t len = kernel_strlen(str);
                while (len --) {
                    *current++ = *str++;
                }
            /* int类型支持 */
            } else if (ch == 'd') {
                int num = va_arg(args, int);
                current = kernel_itoa(current, num, 10);
            /* 16进制的int类型 */
            } else if (ch == 'x') {
                int num = va_arg(args, int);
                current =  kernel_itoa(current, num, 16);
            } else if (ch == 'c') {
                /* 注意, 传参中char转换为int */
                char c = va_arg(args, int);
                *current++ = c;
            } else if( ch == '%') {
                *current++ = '%';
            } else {
                *current++ = '?';
            }
            state = NORMAL;
            break;
        }
    }

    *current = '\0';
}


void panic(const char *file, int line, const char *func, const char *cond) {
    log_printfln("assert failed: %s", cond);
    log_printfln("file:%s", file);
    log_printfln("line:%d", line);
    log_printfln("func:%s", func);
    for(;;) {
        hlt();
    }
}

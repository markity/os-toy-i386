#ifndef KLIB_H
#define KLIB_H

#include "comm/types.h"
#include "stdarg.h"
#include "log.h"

#ifndef RELEASE
#define ASSERT(expr) \
    if(!(expr)) panic(__FILE__, __LINE__, __func__, #expr)
#else
#define ASSERT(expr) ((void)0)
#endif

extern void panic(const char *file, int line, const char *func, const char *cond);

extern void kernel_strcpy(char *dest, const char *src);

extern int kernel_str_equal(const char *s1, const char *s2);

extern uint32_t kernel_strlen(const char* s);

extern char *kernel_itoa(char *buf, int num, int base);

extern void kernel_sprintf(char *buf, const char *fmt, ...);

extern void kernel_memcpy(void *dest, const void *src, uint32_t size);

extern int kernel_mem_equal(const void *d1, const void *d2, uint32_t size);

extern void kernel_memset(void *dest, uint8_t c, uint32_t size);

extern void kernel_vsprintf(char *buf, const char *fmt, va_list args);


#endif
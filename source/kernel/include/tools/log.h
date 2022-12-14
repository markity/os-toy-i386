#ifndef LOG_H
#define LOG_H

#include "comm/cpuistr.h"
#include "stdarg.h"
#include "tools/klib.h"
#include "cpu/irq.h"

extern void log_init(void);
extern void log_println(const char* msg);
extern void log_printfln(const char* fmt, ...);

#endif
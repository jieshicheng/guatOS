#ifndef __KERNEL_STDIO_H
#define __KERNEL_STDIO_H

#include "stdint.h"

typedef char* va_list;


uint32_t printf(const char *format, ...);


#endif

#ifndef __KERNEL_SYSCALL_INIT_H
#define __KERNEL_SYSCALL_INIT_H

#include "stdint.h"


void syscall_init(void);
uint32_t sys_getpid(void);
void sys_putchar(char ch);

#endif

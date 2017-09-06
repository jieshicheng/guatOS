#ifndef __KERNEL_SYSCALL_H
#define __KERNEL_SYSCALL_H

#include "stdint.h"

enum SYSCALL_NR
{
	SYS_GETPID;
};

uint32_t getpid(void);

#endif
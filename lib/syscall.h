#ifndef __KERNEL_SYSCALL_H
#define __KERNEL_SYSCALL_H

#include "stdint.h"

enum SYSCALL_NR
{
	SYS_GETPID,
	SYS_WRITE，
	SYS_MALLOC,
	SYS_FREE
};

uint32_t getpid(void);
uint32_t write(char *str);
void *malloc(uint32_t size);
void free(void *ptr);

#endif
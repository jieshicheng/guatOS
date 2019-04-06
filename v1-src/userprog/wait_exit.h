#ifndef __KERNEL_WAITEXIT_H
#define __KERNEL_WAITEXIT_H


#include "stdint.h"

void sys_exit(int32_t status);
pid_t sys_wait(int32_t *status);


#endif
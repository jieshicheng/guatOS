#ifndef __KERNEL_TIMER_H
#define __KERNEL_TIMER_H

#include "stdint.h"


/**
 *	interface function
 */
void timer_init();
void mtime_sleep(uint32_t m_seconds);

#endif
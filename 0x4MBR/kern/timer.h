#ifndef __KERNEL_TIMER_H
#define __KERNEL_TIMER_H

#include "stdint.h"

static void frequency_set(uint8_t , uint8_t , uint8_t , uint8_t , uint16_t );
void timer_init();

#endif
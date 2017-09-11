#ifndef __KERNEL_TIMER_H
#define __KERNEL_TIMER_H

#include "stdint.h"


/**
 *	interface function
 */
void timer_init();
void mtime_sleep(uint32_t m_seconds);


/**
 *	inside function
 */
static void frequency_set(uint8_t , uint8_t , uint8_t , uint8_t , uint16_t );
static void intr_timer_handler(void);
static void ticks_to_sleep(uint32_t sleep_ticks);

#endif
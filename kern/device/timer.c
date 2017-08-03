#include "timer.h"
#include "io.h"
#include "print.h"
#include "stdint.h"

static void frequency_set(uint8_t counter_port, uint8_t counter_no,
						  uint8_t rwl, uint8_t counter_mode, uint16_t counter_value)
{
	outb(PIT_CONTROL_PORT, (uint8_t)(counter_no << 6 | rwl << 4 | counter_mode << 1));
	outb(counter_port, (uint8_t)counter_value);
	outb(counter_port, (uint8_t)counter_value >> 8);
}

void timer_init()
{
	put_str("timer_init start\n");
	frequency_set(COUNTER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER0_MODE, COUNTER0_VALUE);
	put_str("timer_init end\n");
}
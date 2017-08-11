#include "timer.h"
#include "io.h"
#include "print.h"
#include "stdint.h"
#include "thread.h"
#include "interrupt.h"

uint32_t ticks = 0;

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
	register_handler(0x20, intr_timer_handler);
	put_str("timer_init end\n");
}

static void intr_timer_handler(void)
{
	struct task_struct *cur_thread = running_thread();
	ASSERT(cur_thread->stack_magic == 0x19870916);

	cur_thread->elapsed_ticks++;
	ticks++;
	if ( cur_thread->ticks == 0 ) {
		schedule();
	}
	else {
		cur_thread->ticks--;
	}
}


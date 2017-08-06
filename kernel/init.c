#include "print.h"
#include "interrupt.h"
#include "timer.h"
#include "memory.h"

void init_all()
{
	put_str("init_all\n");
	idt_init();
	timer_init();
	mem_init();
}

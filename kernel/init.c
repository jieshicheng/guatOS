/**
 *	对应组件初始化工作
 *
 *
 *
 */

#include "print.h"
#include "interrupt.h"
#include "timer.h"
#include "memory.h"
#include "thread.h"
#include "sync.h"
#include "console.h"
#include "keyboard.h"
#include "tss.h"
#include "syscall-init.h"
#include "ide.h"

void init_all()
{
	put_str("init_all\n");
	idt_init();
	mem_init();
	thread_init();
	timer_init();
	console_init();
	keyboard_init();
	tss_init();
	syscall_init();
	ide_init();
}

#include "keyboard.h"
#include "global.h"
#include "stdint.h"
#include "print.h"
#include "io.h"
#include "interrupt.h"

static void intr_keyboard_handler(void)
{
	put_char('K');
	inb(KBD_BUF_PORT);
	return ;
}

void keyboard_init()
{
	put_str(".  keyboard init start\n");
	register_handler(0x21, intr_keyboard_handler);
	put_str(".  keyboard init done\n");
}
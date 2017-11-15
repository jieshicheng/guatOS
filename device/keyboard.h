#ifndef __KERNEL_KEYBOARD_H
#define __KERNEL_KEYBOARD_H

extern struct ioqueue kbd_buf;

#define KBD_BUF_PORT 0x60

void keyboard_init();

#endif

#ifndef __KERNEL_CONSOLE_H
#define __KERNEL_CONSOLE_H

#include "stdint.h"

void console_init();
void console_acquire();
void console_release();
void console_put_str(char *msg);
void console_put_char(uint8_t ch);
void console_put_int(uint32_t num);


#endif
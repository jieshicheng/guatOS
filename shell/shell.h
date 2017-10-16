#ifndef __KERNEL_SHELL_H
#define __KERNEL_SHELL_H

#include "stdint.h"

void my_shell(void);
static void readline(char *buf, int32_t count);
void print_prompt(void);
static int32_t cmd_parse(char *cmd_str, char **argv, char token);

#endif
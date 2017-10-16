#ifndef __KERNEL_BUILDIN_CMD_H
#define __KERNEL_BUILDIN_CMD_H

#include "stdint.h"
#include "global.h"

void make_clear_abs_path(char *path, char *final_path);
static void wash_path(char *old_abs_path, char *new_abs_path);


#endif
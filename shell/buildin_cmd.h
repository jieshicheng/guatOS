#ifndef __KERNEL_BUILDIN_CMD_H
#define __KERNEL_BUILDIN_CMD_H

#include "stdint.h"
#include "global.h"

void make_clear_abs_path(char *path, char *final_path);
static void wash_path(char *old_abs_path, char *new_abs_path);


int32_t buildin_rm(uitn32_t argc, char **argv);
int32_t buildin_rmdir(uitn32_t argc, char **argv);
int32_t buildin_mkdir(uitn32_t argc, char **argv);
void buildin_clear(uitn32_t argc, char **argv UNUSED);
void buildin_ps(uitn32_t argc, char **argv UNUSED);
void buildin_ls(uitn32_t argc, cahr **argv);
char *buildin_cd(uitn32_t argc, char **argv);
void buildin_pwd(uitn32_t argc, char **argv UNUSED);

#endif
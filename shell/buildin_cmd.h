#ifndef __KERNEL_BUILDIN_CMD_H
#define __KERNEL_BUILDIN_CMD_H

#include "stdint.h"
#include "global.h"

void make_clear_abs_path(char *path, char *final_path);
static void wash_path(char *old_abs_path, char *new_abs_path);


int32_t buildin_rm(uint32_t argc, char **argv);
int32_t buildin_rmdir(uint32_t argc, char **argv);
int32_t buildin_mkdir(uint32_t argc, char **argv);
void buildin_clear(uint32_t argc, char **argv UNUSED);
void buildin_ps(uint32_t argc, char **argv UNUSED);
void buildin_ls(uint32_t argc, char **argv);
char *buildin_cd(uint32_t argc, char **argv);
void buildin_pwd(uint32_t argc, char **argv UNUSED);

#endif

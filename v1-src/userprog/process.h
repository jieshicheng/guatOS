#ifndef __KERNEL_PROCESS_H
#define __KERNEL_PROCESS_H

#include "thread.h"

#define USER_STACK3_VADDR	(0xc0000000 - 0x1000)
#define USER_VADDR_START	0x8048000

void start_process(void *filename_);
void page_dir_activate(struct task_struct *p_thread);
void process_activate(struct task_struct *p_thread);
uint32_t *create_page_dir(void);
void create_user_vaddr_bitmap(struct task_struct *user_prog);
void process_execute(void *filename, char *name);

#endif
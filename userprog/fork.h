#ifndef __KERNEL_FORK_H
#define __KERNEL_FORK_H

#include "stdint.h"
#include "global.h"


static int32_t copy_pcb_vaddrbitmap_stack0(struct task_struct *child_thread, 
											struct task_struct *parent_thread);

static void copy_body_stack3(struct task_struct *child_thread, 
								struct task_struct *parent_thread, void *buf_page);
#endif
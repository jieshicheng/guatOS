#ifndef __KERNEL_FORK_H
#define __KERNEL_FORK_H

#include "stdint.h"
#include "global.h"
#include "thread.h"

static int32_t copy_pcb_vaddrbitmap_stack0(struct task_struct *child_thread, 
											struct task_struct *parent_thread);

static void copy_body_stack3(struct task_struct *child_thread, 
								struct task_struct *parent_thread, void *buf_page);

static int32_t build_child_stack(struct task_struct *child_thread);
static void updata_inode_open(struct task_struct *thread);
static int32_t copy_process(struct task_struct *child_thread, struct task_struct *parent_thread);
pid_t sys_fork(void);


#endif

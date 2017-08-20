#ifndef __KERNEL_THREAD_H
#define __KERNEL_THREAD_H

#include "stdint.h"
#include "list.h"

typedef void thread_func(void *);

enum task_status
{
	TASK_RUNNING,
	TASK_READY,
	TASK_BLOCKED,
	TASK_WAITING,
	TASK_HANGING,
	TASK_DIED
};

struct intr_stack
{
	uint32_t vec_no;
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t esp_dummy;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;
	uint32_t gs;
	uint32_t fs;
	uint32_t es;
	uint32_t ds;

	uint32_t err_code;
	void (*eip)(void);
	uint32_t cs;
	uint32_t eflags;
	void *esp;
	uint32_t ss;
};

struct thread_stack
{
	uint32_t ebp;
	uint32_t ebx;
	uint32_t edi;
	uint32_t esi;

	void (*eip)(thread_func *func, void *func_arg);

	void *unused_retaddr;
	thread_func *function;
	void *fun_arg;
};


struct task_struct
{
	uint32_t *self_kstack;
	enum task_status status;
	char name[16];
	uint8_t priority;
	
	int8_t ticks;
	uint32_t elapsed_ticks;
	struct list_elem general_tag;
	struct list_elem all_list_tag;
	uint32_t *pgdir;
	struct virtual_addr userprog_vaddr;
	uint32_t stack_magic;
};

static void make_main_thread(void);
struct task_struct *running_thread();
void schedule();
void thread_init(void);

void thread_block(enum task_status stat);
void thread_unblock(struct task_struct *pthread);

static void kernel_thread(thread_func *function, void *func_arg);
void thread_create(struct task_struct *pthread, thread_func function, void *func_arg);
void init_thread(struct task_struct *pthread, char *name, int prio);
struct task_struct *thread_start(char *name, int prio, thread_func function, void *func_arg);

#endif

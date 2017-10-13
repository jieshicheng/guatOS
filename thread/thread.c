#include "stdint.h"
#include "global.h"
#include "thread.h"
#include "string.h"
#include "memory.h"
#include "list.h"
#include "interrupt.h"
#include "print.h"
#include "debug.h"
#include "process.h"


// list struct of ready state and all thread
struct list thread_ready_list;
struct list thread_all_list;

// avoid lock to distribute pid_t
struct lock pid_lock;

// free thread when CPU is nothing to do
struct task_struct *idle_thread;
// main thread. make up by main function
struct task_struct *main_thread;

// a variable
static struct list_elem *thread_tag;

// outboard function come from switch.s
extern void switch_to(struct task_struct *cur, struct task_struct *next);

/**
 *	free thread. when CPU is nothing to do
 *	then run it
 */
static void idle(void *arg UNUSED)
{
	while (1) {
		thread_block(TASK_BLOCKED);
		asm volatile ("sti; hlt" : : : "memory");
	}
}

/**
 *	volunteer to give up the CPU
 *	and schedule the others thread
 */
void thread_yield(void)
{
	struct task_struct *cur = running_thread();
	enum intr_status old_status = intr_disable();
	ASSERT(!elem_find(&thread_ready_list, &cur->general_tag));
	list_append(&thread_ready_list, &cur->general_tag);
	schedule();
	intr_set_status(old_status);
}

/**
 *	allocate pid when thread is creating
 */
static pid_t allocate_pid(void)
{
	static pid_t next_pid = 0;
	lock_acquire(&pid_lock);
	next_pid++;
	lock_release(&pid_lock);
	return next_pid;
}

/**
 *	get running thread's PCB begin address
 */
struct task_struct *running_thread()
{
	uint32_t esp;
	asm volatile ("mov %%esp, %0" : "=g"(esp));
	return (struct task_struct *)(esp & 0xfffff000);
}

/**
 *	every thread that be run must begin here
 *	kernel_thread forward to run function
 */
static void kernel_thread(thread_func *function, void *func_arg)
{
	intr_enable();
	function(func_arg);
}

/**
 *	set the real function of thread.
 *		prefect the environment of running
 */
void thread_create(struct task_struct *pthread, thread_func function, void *func_arg)
{
	pthread->self_kstack -= sizeof(struct intr_stack);
	pthread->self_kstack -= sizeof(struct thread_stack);

	struct thread_stack *kthread_stack = (struct thread_stack *)pthread->self_kstack;
	kthread_stack->eip = kernel_thread;
	kthread_stack->function = function;
	kthread_stack->fun_arg = func_arg;
	kthread_stack->ebp = kthread_stack->ebx = kthread_stack->esi = kthread_stack->edi = 0;
}

/**
 *	set the member of PCB
 */
void init_thread(struct task_struct *pthread, char *name, int prio)
{
	memset(pthread, 0, sizeof(*pthread));
	pthread->pid = allocate_pid();
	strcpy(pthread->name, name);
	if ( pthread == main_thread ) {
		pthread->status = TASK_RUNNING;
	}
	else {
		pthread->status = TASK_READY;
	}

	
	pthread->ticks = prio;
	pthread->elapsed_ticks = 0;
	pthread->pgdir = NULL;
	pthread->priority = prio;
	pthread->self_kstack = (uint32_t *)((uint32_t)pthread + PAGE_SIZE);
	
	pthread->fd_table[0] = 0;
	pthread->fd_table[1] = 1;
	pthread->fd_table[2] = 2;
	uint8_t fd_idx = 3;
	while( fd_idx < MAX_FILES_OPEN_PER_PROC ) {
		pthread->fd_table[fd_idx] = -1;
		fd_idx++;
	}
	pthread->cwd_inode_nr = 0;
	pthread->parent_pid = -1;
	pthread->stack_magic = 0x19870916;
}

/**
 *	create a new thread by two steps
 *		1. init_thread()
 *		2. thread_create()
 *		then append it to list
 */
struct task_struct *thread_start(char *name, int prio, thread_func function, void *func_arg)
{
	struct task_struct *thread = get_kernel_pages(1);
	init_thread(thread, name, prio);
	thread_create(thread, function, func_arg);

	ASSERT( !elem_find(&thread_ready_list, &thread->general_tag) );
	list_append(&thread_ready_list, &thread->general_tag);

	ASSERT( !elem_find(&thread_all_list, &thread->all_list_tag) );
	list_append(&thread_all_list, &thread->all_list_tag);

	return thread;
}

/**
 *	make the main function to be the main thread
 */
static void make_main_thread(void)
{
	main_thread = running_thread();
	init_thread(main_thread, "main", 31);

	ASSERT( !elem_find(&thread_all_list, &main_thread->all_list_tag) );
	list_append(&thread_all_list, &main_thread->all_list_tag);
}

/**
 *	schedule others thread to running
 *
 */
void schedule()
{
	ASSERT(intr_get_status() == INTR_OFF);

	struct task_struct *cur = running_thread();
	if ( cur->status == TASK_RUNNING ) {
		ASSERT( !elem_find(&thread_ready_list, &cur->general_tag));
		list_append(&thread_ready_list, &cur->general_tag);
		cur->ticks = cur->priority;
		cur->status = TASK_READY;
	}
	else {
		// empty
	}

	if (list_empty(&thread_ready_list)) {
		thread_unblock(idle_thread);
	}
	thread_tag = NULL;
	thread_tag = list_pop(&thread_ready_list);
	struct task_struct *next = elem2entry(struct task_struct, general_tag, thread_tag);
	next->status = TASK_RUNNING;
	
	process_activate(next);

	switch_to(cur, next);
}

/**
 *	init the everything about thread
 *		made it can use
 */
void thread_init(void) 
{
	put_str("thread init start \n");
	list_init(&thread_ready_list);
	list_init(&thread_all_list);
	lock_init(&pid_lock);
	make_main_thread();
	idle_thread = thread_start("idle", 10, idle, NULL);
	put_str("thread init done\n");
}

/**
 *	block a thread to state by given parameter
 */
void thread_block(enum task_status stat)
{
	ASSERT(stat == TASK_BLOCKED || stat == TASK_WAITING || stat == TASK_HANGING);
	enum intr_status old_status = intr_disable();
	struct task_struct *cur_thread = running_thread();
	cur_thread->status = stat;
	schedule();
	intr_set_status(old_status);
}

/**
 *	unblock a thread by given PCB
 */
void thread_unblock(struct task_struct *pthread)
{
	enum intr_status old_status = intr_disable();
	ASSERT( pthread->status == TASK_BLOCKED || pthread->status == TASK_WAITING || pthread->status == TASK_HANGING);
	if ( pthread->status != TASK_READY ) {
		ASSERT(!elem_find(&thread_ready_list, &pthread->general_tag));
		list_push(&thread_ready_list, &pthread->general_tag);
		pthread->status = TASK_READY;
	}
	intr_set_status(old_status);
}

pid_t fork_pid()
{
	return allocate_pid();
}




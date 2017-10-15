#include "fork.h"
#include "global.h"
#include "stdint.h"
#include "thread.h"
#include "string.h"
#include "memory.h"
#include "bitmap.h"
#include "process.h"
#include "file.h"
#include "debug.h"
#include "interrupt.h"
#include "list.h"

extern void intr_exit(void);
extern struct file file_table[MAX_FILE_OPEN];
extern struct list thread_ready_list;
extern struct list thread_all_list;

static int32_t copy_pcb_vaddrbitmap_stack0(struct task_struct *child_thread, 
											struct task_struct *parent_thread)
{
	memcpy(child_thread, parent_thread, PAGE_SIZE);
	child_thread->pid = fork_pid();
	child_thread->elapsed_ticks = 0;
	child_thread->status = TASK_READY;
	child_thread->ticks = child_thread->priority;
	child_thread->parent_pid = parent_thread->pid;
	child_thread->general_tag.prev = child_thread->general_tag.next = NULL;
	child_thread->all_list_tag.prev = child_thread->all_list_tag.next = NULL;
	block_desc_init(child_thread->u_block_desc);
	uint32_t bitmap_pg_cnt = DIV_ROUND_UP((0xc0000000 - USER_VADDR_START) / PAGE_SIZE / 8, PAGE_SIZE);
	void *vaddr_btmp = get_kernel_pages(bitmap_pg_cnt);
	memcpy(vaddr_btmp, child_thread->userprog_vaddr.vaddr_bitmap.bits, bitmap_pg_cnt * PAGE_SIZE);
	child_thread->userprog_vaddr.vaddr_btmp.bits = vaddr_btmp;
	ASSERT(strlen(child_thread->name) < 11);
	strcat(child_thread->name, "_fork");
	return 0;
}


static void copy_body_stack3(struct task_struct *child_thread, 
								struct task_struct *parent_thread, void *buf_page)
{
	uint8_t *vaddr_btmp = parent_thread->userprog_vaddr.vaddr_bitmap.bits;
	uint32_t btmp_bytes_len = parent_thread->userprog_vaddr.vaddr_bitmap.btmp_bytes_len;
	uint32_t vaddr_start = parent_thread->userprog_vaddr.vaddr_bitmap.vaddr_start;
	uint32_t idx_byte = 0;
	uint32_t idx_bit = 0;
	uint32_t prog_vaddr = 0;

	while( idx_byte < btmp_bytes_len ) {
		if( vaddr_btmp[idx_byte] ) {
			idx_bit = 0;
			while( idx_bit < 8 ) {
				if( (BITMAP_MASK << idx_bit) & vaddr_btmp[idx_byte] ) {
					prog_vaddr = (idx_byte * 8 + idx_bit) * PAGE_SIZE + vaddr_start;
					memcpy(buf_page, (void *)prog_vaddr, PAGE_SIZE);
					page_dir_activate(child_thread);
					get_a_page_without_opvaddrbitmap(PF_USER, prog_vaddr);
					memcpy((void *)prog_vaddr, buf_page, PAGE_SIZE);
					page_dir_activate(parent_thread);
				}
				idx_bit++;
			}
		}
		idx_byte++;
	}
}


static int32_t build_child_stack(struct task_struct *child_thread)
{
	struct intr_stack *intr_0_stack = (struct intr_stack *)((uint32_t)child_thread + 
										PAGE_SIZE - sizeof(struct intr_stack));
	intr_0_stack->eax = 0;
	uint32_t *ret_addr_in_thread_stack = (uint32_t *)intr_0_stack - 1;
	uint32_t *esi_ptr_in_thread_stack = (uint32_t *)intr_0_stack - 2;
	uint32_t *edi_ptr_in_thread_stack = (uint32_t *)intr_0_stack - 3;
	uint32_t *ebx_ptr_in_thread_stack = (uint32_t *)intr_0_stack - 4;
	uint32_t *ebp_ptr_in_thread_stack = (uint32_t *)intr_0_stack - 5;

	*ret_addr_in_thread_stack = (uint32_t *)intr_exit;
	*ebp_ptr_in_thread_stack = *ebx_ptr_in_thread_stack =
	*edi_ptr_in_thread_stack = esi_ptr_in_thread_stack = 0;

	child_thread->self_kstack = ebp_ptr_in_thread_stack;
	return 0;
}

static void updata_inode_open(struct task_struct *thread)
{
	int32_t local_fd = 3, global_fd = 0;
	while( local_fd < MAX_FILES_OPEN_PER_PROC) {
		global_fd = thread->fd_table[local_fd];
		ASSERT(global_fd < MAX_FILE_OPEN);
		if( global_fd != -1 ) {
			file_table[global_fd].fd_inode->i_open_cnts++;
		}
		local_fd++;
	}
}

static int32_t copy_process(struct task_struct *child_thread, struct task_struct *parent_thread)
{	
	void *buf_page = get_kernel_pages(1);
	if( buf_page == NULL ) {
		return -1;
	}
	if( copy_pcb_vaddrbitmap_stack0(child_thread, parent_thread) == -1 ) {
		return -1;
	}
	child_thread->pgdir = create_page_dir();
	if( child_thread->pgdir == NULL ) {
		return -1;
	}
	copy_body_stack3(child_thread, parent_thread, buf_page);
	build_child_stack(child_thread);
	updata_inode_open(child_thread);
	mfree_page(PF_KERNEL, buf_page, 1);
	return 0;
}

pid_t sys_fork(void)
{
	struct task_struct *parent_thread = running_thread();
	struct task_struct *child_thread = get_kernel_pages(1);
	if( child_thread == NULL ) {
		return -1;
	}
	ASSERT(INTR_OFF == intr_get_status() && parent_thread->pgdir != NULL);
	if( copy_process(child_thread, parent_thread) == -1 ) {
		return -1;
	}
	ASSERT(!elem_find(&thread_ready_list, &child_thread->general_tag));
	list_append(&thread_ready_list, &child_thread->general_tag);
	ASSERT(!elem_find(&thread_all_list, &child_thread->all_list_tag));
	list_append(&thread_all_list, &child_thread->all_list_tag);

	return child_thread->pid;
}









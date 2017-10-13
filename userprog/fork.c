#include "fork.h"
#include "global.h"
#include "stdint.h"
#include "thread.c"
#include "string.h"
#include "memory.h"
#include "bitmap.h"
#include "process.h"

extern void intr_exit(void);

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

}















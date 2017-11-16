#include "stdint.h"
#include "global.h"
#include "thread.h"
#include "memory.h"
#include "bitmap.h"
#include "list.h"
#include "fs.h"

extern struct list thread_all_list;
extern struct list thread_ready_list;

static void release_prog_resource(struct task_struct *release_thread)
{
	uint32_t *pgdir_vaddr = release_thread->pgdir;
	uint16_t user_pde_nr = 768, pde_idx = 0;
	uint32_t pde = 0;
	uint32_t *v_pde_ptr = NULL;

	uint16_t user_pte_nr = 1024, pte_idx = 0;
	uint32_t pte = 0;
	uint32_t *v_pte_ptr = NULL;

	uint32_t *first_pte_vaddr_in_pde = NULL;
	uint32_t pg_phy_addr = 0;

	while( pde_idx < user_pde_nr ) {
		v_pde_ptr = pgdir_vaddr + pde_idx;
		pde = *v_pde_ptr;
		if( pde & 0x00000001 ) {
			first_pte_vaddr_in_pde = pte_ptr(pde_idx * 0x400000);
			pte_idx = 0;
			while( pte_idx < user_pte_nr ) {
				v_pte_ptr = first_pte_vaddr_in_pde + pte_idx;
				pte = *v_pte_ptr;
				if( pte * 0x00000001 ) {
					pg_phy_addr = pte & 0xfffff000;
					free_a_phy_page(pg_phy_addr);
				}
				pte_idx++;
			}
			pg_phy_addr = pde & 0xfffff000;
			free_a_phy_page(pg_phy_addr);
		}
		pde_idx++;
	}

	uint32_t bitmap_pg_cnt = (release_thread->userprog_vaddr.vaddr_bitmap.btmp_bytes_len) / PAGE_SIZE;
	uint8_t *user_vaddr_pool_bitmap = release_thread->userprog_vaddr.vaddr_bitmap.bits;
	mfree_page(PF_KERNEL, user_vaddr_pool_bitmap, bitmap_pg_cnt);

	uint8_t fd_idx = 3;
	while( fd_idx < MAX_FILES_OPEN_PER_PROC ) {
		if( release_thread->fd_table[fd_idx] != -1 ) {
			sys_close(fd_idx);
		}
		fd_idx++;
	}
}


static enum bool find_child(struct list_elem *pelem, int32_t ppid)
{
	struct task_struct *pthread = elem2entry(struct task_struct, all_list_tag, pelem);
	if( pthread->parent_pid == ppid )
		return true;
	return false;
}

static enum bool find_hanging_child(struct list_elem *pelem, int32_t ppid)
{
	struct task_struct *pthread = elem2entry(struct task_struct, all_list_tag, pelem);
	if( pthread->parent_pid == ppid && pthread->status == TASK_HANGING ) {
		return true;
	}
	return false;
}

static enum bool init_adopt_a_child(struct list_elem *pelem, int32_t pid)
{
	struct task_struct *pthread = elem2entry(struct task_struct, all_list_tag, pelem);
	if( pthread->parent_pid == pid ) {
		pthread->parent_pid = 1;
	}
	return false;
}


pid_t sys_wait(int32_t *status)
{
	struct task_struct *parent_thread = running_thread();
	while( 1 ) {
		struct list_elem *child_elem = list_traversal(&thread_all_list, find_hanging_child, parent_thread->pid);
		if( child_elem != NULL ) {
			struct task_struct *child_thread = elem2entry(struct task_struct, all_list_tag, child_elem); 
			*status = child_thread->exit_status;
			uint16_t child_pid = child_thread->pid;
			thread_exit(child_thread, false);
			return child_pid;
		}
		child_elem = list_traversal(&thread_all_list, find_child, parent_thread->pid);
		if( child_elem == NULL ) {
			return -1;
		}
		else {
			thread_block(TASK_HANGING);
		}
	}
}

void sys_exit(int32_t status)
{
	struct task_struct *child_thread = running_thread();
	child_thread->exit_status = status;
	if( child_thread->parent_pid == -1 ) {
		while(1);
	}
	list_traversal(&thread_all_list, init_adopt_a_child, child_thread->pid);
	release_prog_resource(child_thread);
	struct task_struct *parent_thread = pid2thread(child_thread->parent_pid);
	if( parent_thread->status == TASK_HANGING ) {
		thread_unblock(parent_thread);
	}
	thread_block(TASK_HANGING);
}

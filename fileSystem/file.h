#ifndef __KERNEL_FILE_H
#define __KERNEL_FILE_H

#include "stdint.h"

#define MAX_FILE_OPEN 32

struct file
{
	uint32_t fd_pos;
	uint32_t fd_flag;
	struct inode *fd_inode;
};

enum std_fd
{
	stdin_no,
	stdout_no,
	stderr_no
};

enum bitmap_type
{
	INODE_BITMAP,
	BLOCK_BITMAP
};

/**
 *	interface function
 */
int32_t get_free_slot_in_global(void);
int32_t pcb_fd_install(int32_t global_fd_idx);
int32_t inode_bitmap_alloc(struct partition *part);
int32_t block_bitmap_alloc(struct partition *part);
void bitmap_sync(struct partition *part, uint32_t bit_idx, uint8_t btmp);


/**
 *	inside function
 */


#endif
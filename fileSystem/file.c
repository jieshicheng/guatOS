#include "file.h"
#include "thread.h"
#include "ide.h"
#include "bitmap.h"

struct file file_table[MAX_FILE_OPEN];

int32_t get_free_slot_in_global(void)
{
	uint32_t fd_idx = 3;
	while( fd_idx < MAX_FILE_OPEN ) {
		if( file_table[fd_idx].fd_inode == NULL ) {
			break;
		}
		fd_idx++;
	}
	if( fd_idx == MAX_FILE_OPEN ) {
		printk("exceed max open files\n");
		return -1;
	}
	return fd_idx;
}

int32_t pcb_fd_install(int32_t global_fd_idx)
{
	struct task_struct *cur = running_thread();
	uint8_t local_fd_idx = 3;
	while( local_fd_idx < MAX_FILES_OPEN_PER_PROC ) {
		if( cur->fd_table[local_fd_idx] == -1 ) {
			cur->fd_table[local_fd_idx] = global_fd_idx;
			break;
		}
		local_fd_idx++;
	}
	if( local_fd_idx == MAX_FILES_OPEN_PER_PROC ) {
		printk("exceed max open file_per_proc\n");
		return -1;
	}
	return local_fd_idx;
}

int32_t inode_bitmap_alloc(struct partition *part)
{
	int32_t bit_idx = bitmap_scan(&part->inode_bitmap, 1);
	if( bit_idx == -1 ) {
		return -1;
	}
	bitmap_set(&part->inode_bitmap, bit_idx, 1);
	return bit_idx;
}


int32_t block_bitmap_alloc(struct partition *part)
{
	int32_t bit_idx = bitmap_scan(&part->block_bitmap, 1);
	if( bit_idx == -1 )
		return -1;
	bitmap_set(&part->block_bitmap, bit_idx, 1);
	return (part->sb->data_start_lba + bit_idx);
}


void bitmap_sync(struct partition *part, uint32_t bit_idx, uint8_t btmp)
{
	uint32_t off_sec = bit_idx / 4096;
	uint32_t off_size = off_sec * BLOCK_SIZE;
	uint32_t sec_lba;
	uint8_t *bitmap_off;

	switch( btmp ) {
		case INODE_BITMAP:
			sec_lba = part->sb->inode_bitmap_lba + off_sec;
			bitmap_off = part->inode_bitmap.bits + off_size;
			break;
		case BLOCK_BITMAP:
			sec_lba = part->sb->block_bitmap_lba + off_sec;
			bitmap_off = part->block_bitmap.bits + off_size;
			break;
	}
	ide_wrtie(part->my_disk, sec_lba, bitmap_off, 1);
}













#ifndef __KERNEL_FS_H
#define __KERNEL_FS_H

#define MAX_FILES_PER_PART	4096
#define BITS_PER_SECTOR	4096
#define SECTOR_SIZE		512
#define BLOCK_SIZE		SECTOR_SIZE;


enum file_types
{
	FT_UNKNOWN,
	FT_REGULAR,
	FT_DIRECTORY
};

/**
 *	interface function
 */
void filesys_init();

/**
 *	inside function
 */
static void partition_format(struct partition *part);

#endif

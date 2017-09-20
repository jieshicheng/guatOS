#ifndef __KERNEL_FS_H
#define __KERNEL_FS_H

#include "global.h"
#include "list.h"

#define MAX_FILES_PER_PART	4096
#define BITS_PER_SECTOR	4096
#define SECTOR_SIZE		512
#define BLOCK_SIZE		SECTOR_SIZE;
#define MAX_PATH_LEN 512

enum file_types
{
	FT_UNKNOWN,
	FT_REGULAR,
	FT_DIRECTORY
};

enum oflags
{
	O_RDONLY = 0,
	O_WRONLY = 1,
	O_RDWR = 2,
	O_CREAT = 4
};


struct path_search_record
{
	char searched_path[MAX_PATH_LEN];
	struct dir *parent_dir;
	enum file_types file_type;
};


/**
 *	interface function
 */
void filesys_init();

/**
 *	inside function
 */
static void partition_format(struct partition *part);
static enum bool mount_partition(struct list_elem *pelem, int arg);

static int search_file(const char *pathname, struct path_depth_cnt *searched_record);
int32_t path_depth_cnt(char *pathname);
static char *path_parse(char *pathname, char *name_host);
#endif

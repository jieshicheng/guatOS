#ifndef __KERNEL_FS_H
#define __KERNEL_FS_H

#include "global.h"
#include "list.h"
#include "ide.h"

// 每个分区最多有的文件个数。也就是inode个数
#define MAX_FILES_PER_PART	4096

#define BITS_PER_SECTOR	4096
#define SECTOR_SIZE		512
#define BLOCK_SIZE		SECTOR_SIZE
// 路径名字缓冲区的最长长度
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

enum whence
{
	SEEK_SET = 1,
	SEEK_CUR,
	SEEK_END
};



struct path_search_record
{
	char searched_path[MAX_PATH_LEN];
	struct dir *parent_dir;
	enum file_types file_type;
};

struct stat
{
	uint32_t st_ino;
	uint32_t st_size;
	enum file_type st_fileType;
};


/**
 *	interface function
 */
void filesys_init();
int32_t sys_open(const char *pathname, uint8_t flags);
int32_t sys_close(int32_t fd);
int32_t sys_write(int32_t fd, const void *buf, uint32_t count);
int32_t sys_read(int32_t fd, void *buf, uint32_t count);
int32_t sys_lseek(int32_t fd, int32_t offset, uint8_t whence);
int32_t sys_unlink(const char *pathname) ;
struct dir *sys_opendir(const char *name);
int32_t sys_closedir(struct dir *dir);
struct dir_entry *sys_readdir(struct dir *dir);
void sys_rewinddir(struct dir *dir);
int32_t sys_rmdir(const char *name);
char *sys_getcwd(char *buf, uint32_t size);
int32_t sys_chdir(const char *path);
int32_t sys_stat(const char *path, struct stat *buf);

/**
 *	inside function
 */
static void partition_format(struct partition *part);
static enum bool mount_partition(struct list_elem *pelem, int arg);
static int search_file(const char *pathname, struct path_search_record *searched_record);
int32_t path_depth_cnt(char *pathname);
static char *path_parse(char *pathname, char *name_host);
static uint32_t fd_local2global(uint32_t local_fd);
static int get_child_dir_name(uint32_t p_inode_nr, uint32_t c_inode_nr, char *path, void *io_buf);
static uint32_t get_parent_dir_inode_nr(uint32_t child_inode_nr, void *io_buf);

#endif

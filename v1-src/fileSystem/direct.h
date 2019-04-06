#ifndef __KERNEL_DIRECT_H
#define __KERNEL_DIRECT_H


#include "stdint.h"
#include "fs.h"

#define MAX_FILE_NAME_LEN	16


struct dir
{
	struct inode *inode;
	uint32_t dir_pos; //offset in here
	uint8_t dir_buf[512];
};

struct dir_entry
{
	char name[MAX_FILE_NAME_LEN];
	uint32_t i_no;
	enum file_types f_type;
};

/**
 *	interface function
 */
enum bool sync_dir_entry(struct dir *parent_dir, struct dir_entry *p_de, void *io_buf);
void create_dir_entry(char *name, uint32_t inode_no, uint8_t file_type, struct dir_entry *p_de);
void dir_close(struct dir *dir);
enum bool search_dir_entry(struct partition *part, struct dir *pdir, const char *name, struct dir_entry *dir_e);
struct dir *dir_open(struct partition *part, uint32_t inode_no);
void open_root_dir(struct partition *part);
enum bool delete_dir_entry(struct partition *part, struct dir *pdir, uint32_t inode_no, void *io_buf);
struct dir_entry *dir_read(struct dir *dir);
enum bool dir_is_empty(struct dir *dir);
int32_t dir_remove(struct dir *parent_dir, struct dir *child_dir);

#endif

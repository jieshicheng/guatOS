#ifndef __KERNEL_INODE_H
#define __KERNEL_INODE_H

#include "stdint.h"
#include "global.h"
#include "list.h"
#include "ide.h"

struct inode
{
	uint32_t i_no;   
	uint32_t i_size;	//file or sum of dir_entry size
	uint32_t i_open_cnts;
	enum bool write_deny;
	uint32_t i_sectors[13];
	struct list_elem inode_tag;
};

struct inode_position
{
	enum bool two_sec;
	// 起始扇区
	uint32_t sec_lba;
	// 偏移量
	uint32_t off_size;
};

/**
 *	interface function
 */
void inode_sync(struct partition *part, struct inode *inode, void *io_buf);
struct inode *inode_open(struct partition *part, uint32_t inode_no);
void inode_close(struct inode *inode);
void inode_init(uint32_t inode_no, struct inode *new_inode);

/**
 *	inside function
 */
static void inode_locate(struct partition *part, uint32_t inode_no, struct inode_position *inode_pos);


#endif

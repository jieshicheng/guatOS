#ifndef __KERNEL_INODE_H
#define __KERNEL_INODE_H

#include "stdint.h"
#include "global.h"
#include "list.h"

struct inode
{
	uint32_t i_no;
	uint32_t i_size;
	uint32_t i_open_cnts;
	enum bool write_deny;
	uint32_t i_sectors[13];
	struct list_elem inode_tag;
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
static void inode_locate(struct partition *part, uint32_t inode_no, struct inode_position *inode_pos)


#endif
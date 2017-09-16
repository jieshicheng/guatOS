#ifndef __KERNEL_DIRECT_H
#define __KERNEL_DIRECT_H


#include "stdint.h"

#define MAX_FILE_NAME_LEN	16


struct dir
{
	struct inode *inode;
	uint32_t dir_pos;
	uint8_t dir_buf[512];
};

struct dir_entry
{
	char name[MAX_FILE_NAME_LEN];
	uint32_t i_no;
	enum file_types f_type;
};



#endif
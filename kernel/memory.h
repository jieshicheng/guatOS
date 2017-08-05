#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H

#include "stdint.h"
#include "bitmap.h"

struct virtual_addr
{
	struct bitmap vaddr_bitmap;
	uint32_t vaddr_start;
};

struct pool
{
	struct bitmap pool_bitmap;
	uint32_t phy_addr_start;
	uint32_t pool_size;
};

extern struct pool kernel_pool, user_pool;
void mem_init(void);


#endif
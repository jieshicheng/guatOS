#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H

#include "stdint.h"
#include "bitmap.h"
#include "sync.h"
#include "list.h"


#define PG_P_1 1
#define PG_P_0 0
#define PG_RW_R 0
#define PG_RW_W 2
#define PG_US_S 0
#define PG_US_U 4


#define MEM_BITMAP_BASE 0xc009a000
#define K_HEAP_START 0xc0100000

// function to get given virtual addr's PDE and PTE
#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)


#define DESC_CNT 7

struct mem_block
{
	struct list_elem free_elem;
};

struct mem_block_desc
{
	uint32_t block_size;
	uint32_t blocks_pre_arena;
	struct list free_list;
};


enum pool_flags
{
	PF_KERNEL = 1,
	PF_USER = 2
};


struct virtual_addr
{
	struct bitmap vaddr_bitmap;
	uint32_t vaddr_start;
};

struct pool
{
	struct lock lock;
	struct bitmap pool_bitmap;
	uint32_t phy_addr_start;
	uint32_t pool_size;
};


void block_desc_init(struct mem_block_desc *desc_array);
void *sys_malloc(uint32_t size);
void sys_free(void *ptr);



extern struct pool kernel_pool, user_pool;
void mem_init(void);
static void mem_pool_init(uint32_t all_mem);
static void *vaddr_get(enum pool_flags pf, uint32_t pg_cnt);
uint32_t *pte_ptr(uint32_t vaddr);
uint32_t *pde_ptr(uint32_t vaddr);
static void *palloc(struct pool *m_pool);
static void page_table_add(void *_vaddr, void *_page_phyaddr);
void *get_kernel_pages(uint32_t pg_cnt);
void *get_user_pages(uint32_t pg_cnt);
void *malloc_page(enum pool_flags pf, uint32_t pg_cnt);
void *get_a_page(enum pool_flags pf, uint32_t vaddr);
uint32_t addr_v2p(uint32_t vaddr);
void *get_a_page_without_opvaddrbitmap(enum pool_flags pf, uint32_t vaddr);

#endif

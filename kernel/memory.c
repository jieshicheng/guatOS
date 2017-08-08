/**
 *	内存组成：内核物理内存池
 *			用户物理内存池
 *			内核虚拟内存池
 *	内存池又是靠位图来表示空与否的
 *
 *	这里分配内核内存的时候需要修改内核虚拟内存池，然后修改内核物理内存池
 *	然后在二级页表中建立到物理地址的映射。最后把虚拟地址返回
 */

#include "memory.h"
#include "stdint.h"
#include "print.h"
#include "bitmap.h"


// new include file. edit makefile
//
// ...................
#include "string.h"
#include "debug.h"
#include "global.h"

#define PAGE_SIZE 4096
#define MEM_BITMAP_BASE 0xc009a000
#define K_HEAP_START 0xc0100000

// function to get given virtual addr's PDE and PTE
#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)


// 定义内核，用户态的物理内存池
struct pool kernel_pool, user_pool;

// 定义内核虚拟内存池，因为目前没有用户态的进程，所以没有用户虚拟池
struct virtual_addr kernel_vaddr;

/**
 * 初始化三个内存池
 * 传入参数是整个机器的所有内存大小
 */
static void mem_pool_init(uint32_t all_mem)
{
	put_str("    mem pool init start\n");
	// 已经使用的二级页表占用的内存
	// PDE 加 第一个和769到1022个PTE一共256个页面
	uint32_t page_table_size = PAGE_SIZE * 256;
	uint32_t used_mem = 0x100000 + page_table_size;  // 已经使用的1M加上页表占用的
	uint32_t free_mem = all_mem - used_mem;    // 剩下的内存
	uint16_t free_pages = free_mem / PAGE_SIZE;  // 剩下的多少个页面
	uint16_t kernel_free_pages = free_pages / 2;  // 用户和内核平分剩下的内存
	uint16_t user_free_pages = free_pages - kernel_free_pages;
	uint32_t kbm_length = kernel_free_pages / 8; // 内核，用户内存池位图的长度
	uint32_t ubm_length = user_free_pages / 8;
	uint32_t kp_start = used_mem; // 内核空余内存的起始物理地址
	uint32_t up_start = kp_start + kernel_free_pages * PAGE_SIZE; // 用户空余内存的起始物理地址
	
	// 为内存池里面的相应结构赋值
	kernel_pool.phy_addr_start = kp_start;
	user_pool.phy_addr_start = up_start;
	kernel_pool.pool_size = kernel_free_pages * PAGE_SIZE;
	user_pool.pool_size = user_free_pages * PAGE_SIZE;
	kernel_pool.pool_bitmap.btmp_bytes_len = kbm_length;
	user_pool.pool_bitmap.btmp_bytes_len = ubm_length;

	kernel_pool.pool_bitmap.bits = (void *)MEM_BITMAP_BASE;
	user_pool.pool_bitmap.bits = (void *)(MEM_BITMAP_BASE + kbm_length);

	put_str(".   kernel pool bitmap start: ");
	put_int((int)kernel_pool.pool_bitmap.bits);
	put_str(".   kernel pool phy addr start: ");
	put_int((int)kernel_pool.phy_addr_start);
	put_char('\n');
	put_str(".   user pool bitmap start: ");
	put_int((int)user_pool.pool_bitmap.bits);
	put_str(".   user pool phy addr start: ");
	put_int((int)user_pool.phy_addr_start);
	put_char('\n');

	// 初始化位图
	bitmap_init(&kernel_pool.pool_bitmap);
	bitmap_init(&user_pool.pool_bitmap);

	// 初始化虚拟内存池
	kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kbm_length;
	kernel_vaddr.vaddr_bitmap.bits = (void *)(MEM_BITMAP_BASE + kbm_length + ubm_length);
	kernel_vaddr.vaddr_start = K_HEAP_START;
	bitmap_init(&kernel_vaddr.vaddr_bitmap);
	put_str(".   mem pool init done\n");

}

/**
 *	init function
 *
 */
void mem_init()
{
	put_str("mem_init start\n");
	uint32_t mem_bytes_total = (*(uint32_t *)(0xb00));
	mem_pool_init(mem_bytes_total);
	put_str("mem_init done\n");
}

/**
 *	配置虚拟内存，将虚拟内存池对应的内存位分配好
 *
 */
static void *vaddr_get(enum pool_flags pf, uint32_t pg_cnt)
{
	uint32_t cnt = 0;
	int vaddr_start = 0, bit_idx_start = -1;
	if(pf == PF_KERNEL) {
		bit_idx_start = bitmap_scan(&kernel_vaddr.vaddr_bitmap, pg_cnt);
		if(bit_idx_start == -1) {
			return NULL;
		}
		while(cnt < pg_cnt) {
			bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 1);
		}
		vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PAGE_SIZE;
	}
	else {
		// user memory allocate
		// waitting for complete
	}
	return (void *)vaddr_start;
}

/**
 *	according virtual address to get
 *		page-table-expressing
 */
uint32_t *pte_ptr(uint32_t vaddr)
{
	uint32_t *pte = (uint32_t *)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr) * 4);
	return pte;
}

/**
 *	to get PDE
 */
uint32_t *pde_ptr(uint32_t vaddr)
{
	uint32_t *pde = (uint32_t *)(0xfffff000 + PDE_IDX(vaddr) * 4);
	return pde;
}

/**
 *	在物理内存池中分配一个内存页
 *		成功则返回物理地址，失败则NULL
 */
static void *palloc(struct pool *m_pool)
{
	int bit_idx = bitmap_scan(&m_pool->pool_bitmap, 1);
	if(bit_idx == -1)
		return NULL;
	else {
		bitmap_set(&m_pool->pool_bitmap, bit_idx, 1);
		uint32_t page_phyaddr = ((bit_idx * PAGE_SIZE) + m_pool->phy_addr_start);
		return (void *)page_phyaddr;
	}
}
/**
 *	rebuild page table and page direct
 *		according the virtual addr and physics addr
 */
static void page_table_add(void *_vaddr, void *_page_phyaddr)
{
	uint32_t vaddr = (uint32_t)vaddr, page_phyaddr = (uint32_t)_page_phyaddr;
	uint32_t *pde = pde_ptr(vaddr);
	uint32_t *pte = pte_ptr(vaddr);
	if( *pde & 0x00000001) {
		ASSERT(!(*pte & 0x00000001));
		*pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
	}
	else {
		uint32_t pde_phyaddr = (uint32_t)palloc(&kernel_pool);
		*pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
		memset((void *)((int)pte & 0xfffff000), 0, PAGE_SIZE);
		ASSERT(!(*pte & 0x00000001));
		*pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
	}
}

/**
 *	分配内存的主要函数，一共是三步
 *		1. 从相应的虚拟内存池获取虚拟内存起点
 *		2. 通过palloc从相应的物理内存池获得一页物理内存
 *		3. 调用页表映射函数将虚拟与物理内存映射起来
 */
void *malloc_page(enum pool_flags pf, uint32_t pg_cnt)
{
	ASSERT(pg_cnt > 0 && pg_cnt < 3840);
	void *vaddr_start = vaddr_get(pf, pg_cnt);
	if( vaddr_start == NULL )
		return NULL;
	uint32_t vaddr = (uint32_t)vaddr_start, cnt = pg_cnt;
	struct pool *mem_pool = pf == PF_KERNEL ? &kernel_pool : &user_pool;
	while( cnt-- > 0 ) {
		void *page_phyaddr = palloc(mem_pool);
		if( page_phyaddr == NULL )
			return NULL;
		page_table_add((void *)vaddr, page_phyaddr);
		vaddr += PAGE_SIZE;
	}
	return vaddr_start;
}

/**
 *	内存分配接口函数
 *
 */
void *get_kernel_pages(uint32_t pg_cnt)
{
	void *vaddr = malloc_page(PF_KERNEL, pg_cnt);
	if( vaddr != NULL )
		memset(vaddr, 0, pg_cnt * PAGE_SIZE);
	return vaddr;
}



















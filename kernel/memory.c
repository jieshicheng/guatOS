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
#include "string.h"
#include "debug.h"
#include "global.h"
#include "thread.h"
#include "list.h"
#include "interrupt.h"

// 定义内核，用户态的物理内存池
struct pool kernel_pool, user_pool;

// 定义内核虚拟内存池，因为目前没有用户态的进程，所以没有用户虚拟池
struct virtual_addr kernel_vaddr;

// 定义7中规格的块描述符
struct mem_block_desc k_block_descs[DESC_CNT];

struct arena
{
	struct mem_block_desc *desc;
	uint32_t cnt;
	enum bool large;
};

void block_desc_init(struct mem_block_desc *desc_array)
{
	uint16_t desc_idx, block_size = 16;

	for (desc_idx = 0; desc_idx < DESC_CNT; desc_idx++) {
		desc_array[desc_idx].block_size = block_size;
		desc_array[desc_idx].blocks_pre_arena = (PAGE_SIZE - sizeof(struct arena)) / block_size;
		list_init(&desc_array[desc_idx].free_list);
		block_size *= 2;
	}
}


static struct mem_block *arena2block(struct arena *a, uint32_t idx)
{
	return (struct mem_block *)((uint32_t)a + sizeof(struct arena) + idx * a->desc->block_size);
}

static struct arena *block2arena(struct mem_block *b)
{	
	return (struct arena *)((uint32_t)b & 0xfffff000);
}

void *sys_malloc(uint32_t size)
{
	enum pool_flags pf;
	struct pool *mem_pool;
	uint32_t pool_size;
	struct mem_block_desc *desc;
	struct task_struct *cur_thread = running_thread();

	if (cur_thread->pgdir == NULL) {
		pf = PF_KERNEL;
		pool_size = kernel_pool.pool_size;
		mem_pool = &kernel_pool;
		desc = k_block_descs;
	}
	else {
		pf = PF_USER;
		pool_size = user_pool.pool_size;
		mem_pool = &user_pool;
		desc = cur_thread->u_block_desc;
	}

	if (!(size > 0 && size < pool_size))
		return NULL;
	struct arena *a;
	struct mem_block *b;
	lock_acquire(&mem_pool->lock);

	if (size > 1024) {
		uint32_t page_cnt = DIV_ROUND_UP(size + sizeof(struct arena), PAGE_SIZE);
		a = malloc_page(pf, page_cnt);

		if (a != NULL) {
			memset(a, 0, page_cnt * PAGE_SIZE);
			a->desc = NULL;
			a->cnt = page_cnt;
			a->large = true;
			lock_release(&mem_pool->lock);
			return (void *)(a + 1);
		}
		else {
			lock_release(&mem_pool->lock);
			return NULL;
		}
	}
	else {
		uint8_t desc_idx;

		for (desc_idx = 0; desc_idx < DESC_CNT; desc_idx++) {
			if (size <= desc[desc_idx].block_size)
				break;
		}
		if (list_empty(&desc[desc_idx].free_list)) {
			a = malloc_page(pf, 1);
			if (a == NULL) {
				lock_release(&mem_pool->lock);
				return NULL;
			}
			memset(a, 0, PAGE_SIZE);
			a->desc = &desc[desc_idx];
			a->cnt = desc[desc_idx].blocks_pre_arena;
			a->large = false;

			uint32_t block_idx;
			enum intr_status old_status = intr_disable();

			for (block_idx = 0; block_idx < desc[desc_idx].blocks_pre_arena; block_idx++) {
				b = arena2block(a, block_idx);
				ASSERT(!elem_find(&a->desc->free_list, &b->free_elem));
				list_append(&a->desc->free_list, &b->free_elem);
			}
			intr_set_status(old_status);
		}
		b = elem2entry(struct mem_block, free_elem, list_pop(&(desc[desc_idx].free_list)));
		memset(b, 0, desc[desc_idx].block_size);

		a = block2arena(b);
		a->cnt--;
		lock_release(&mem_pool->lock);
		return (void *)b;
	}

}


void pfree(uint32_t pg_phy_addr)
{
	struct pool *mem_pool;
	uint32_t bit_idx = 0;
	if (pg_phy_addr >= user_pool.phy_addr_start) {
		mem_pool = &user_pool;
		bit_idx = (pg_phy_addr - user_pool.phy_addr_start) / PAGE_SIZE;
	}
	else {
		mem_pool = &kernel_pool;
		bit_idx = (pg_phy_addr - kernel_pool.phy_addr_start) / PAGE_SIZE;
	}
	bitmap_set(&mem_pool->pool_bitmap, bit_idx, 0);
}


static void page_table_pte_remove(uint32_t vaddr)
{
	uint32_t *pte = pte_ptr(vaddr);
	*pte &= ~PG_P_1;
	asm volatile ("invlpg %0" : : "m"(vaddr) : "memory");
}

static void vaddr_remove(enum pool_flags pf, void *_vaddr, uint32_t pg_cnt)
{
	uint32_t bit_idx_start = 0, vaddr = (uint32_t)_vaddr, cnt = 0;
	if (pf == PF_KERNEL) {
		bit_idx_start = (vaddr - kernel_vaddr.vaddr_start) / PAGE_SIZE;
		while (cnt < pg_cnt) {
			bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 0);
		}
	}
	else {
		struct task_struct *cur_thread = running_thread();
		bit_idx_start = (vaddr - cur_thread->userprog_vaddr.vaddr_start) / PAGE_SIZE;
		while (cnt < pg_cnt) {
			bitmap_set(&cur_thread->userprog_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 0);
		}
	}
}

void mfree_page(enum pool_flags pf, void *_vaddr, uint32_t pg_cnt)
{
	uint32_t pg_phy_addr;
	uint32_t vaddr = (uint32_t)_vaddr;
	uint32_t page_cnt = 0;

	ASSERT(pg_cnt >= 1 && ((vaddr % PAGE_SIZE) == 0));
	pg_phy_addr = addr_v2p(vaddr);
	ASSERT((pg_phy_addr % PAGE_SIZE) == 0 && pg_phy_addr >= 0x102000);

	if (pg_phy_addr >= user_pool.phy_addr_start) {
		vaddr -= PAGE_SIZE;
		while (page_cnt < pg_cnt) {
			vaddr += PAGE_SIZE;
			pg_phy_addr = addr_v2p(vaddr);
			ASSERT((pg_phy_addr % PAGE_SIZE) == 0 && pg_phy_addr >= user_pool.phy_addr_start);
			pfree(pg_phy_addr);
			page_table_pte_remove(vaddr);
			page_cnt++;
		}
		vaddr_remove(pf, _vaddr, pg_cnt);
	}
	else {
		vaddr -= PAGE_SIZE;
		while (page_cnt < pg_cnt) {
			vaddr += PAGE_SIZE;
			pg_phy_addr = addr_v2p(vaddr);
			ASSERT((pg_phy_addr % PAGE_SIZE) == 0 && 
					pg_phy_addr >= kernel_pool.phy_addr_start &&
					pg_phy_addr < user_pool.phy_addr_start);
			pfree(pg_phy_addr);
			page_table_pte_remove(vaddr);
			page_cnt++;
		}
		vaddr_remove(pf, _vaddr, pg_cnt);
	}
}


void sys_free(void *ptr)
{
	if (ptr == NULL) {
		return ;
	}
	enum pool_flags pf;
	struct pool *mem_pool;

	if (running_thread()->pgdir == NULL) {
		ASSERT((uint32_t)ptr >= K_HEAP_START);
		pf = PF_KERNEL;
		mem_pool = &kernel_pool;
	}
	else {
		pf = PF_USER;
		mem_pool = &user_pool;
	}

	lock_acquire(&mem_pool->lock);
	struct mem_block *b = ptr;
	struct arena *a = block2arena(b);

	ASSERT(a->large == true || a->large == false);
	if (a->desc == NULL && a->large == true) {
		mfree_page(pf, a, a->cnt);
	}
	else {
		list_append(&a->desc->free_list, &b->free_elem);

		if (++a->cnt == a->desc->blocks_pre_arena) {
			uint32_t block_idx;
			for (block_idx = 0; block_idx < a->desc->blocks_pre_arena; block_idx++) {
				struct mem_block *b = arena2block(a, block_idx);
				ASSERT(elem_find(&a->desc->free_list, &b->free_elem));
				list_remove(&b->free_elem);
			}
			mfree_page(pf, a, 1);
		}
	}
	lock_release(&mem_pool->lock);
}


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
	
	lock_init(&kernel_pool.lock);
	lock_init(&user_pool.lock);


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
	uint32_t mem_bytes_total = (*(uint32_t *)0xb00);
	mem_pool_init(mem_bytes_total);
	block_desc_init(k_block_descs);
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
		struct task_struct *cur = running_thread();
		bit_idx_start = bitmap_scan(&cur->userprog_vaddr.vaddr_bitmap, pg_cnt);
		if ( bit_idx_start == -1 ) {
			return NULL;
		}

		while ( cnt < pg_cnt ) {
			bitmap_set(&cur->userprog_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 1);
		}
		vaddr_start = cur->userprog_vaddr.vaddr_start + bit_idx_start * PAGE_SIZE;
		ASSERT((uint32_t)vaddr_start < (0xc0000000 - PAGE_SIZE));

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
	uint32_t vaddr = (uint32_t)_vaddr, page_phyaddr = (uint32_t)_page_phyaddr;
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
 *	内核内存分配接口函数
 *
 */
void *get_kernel_pages(uint32_t pg_cnt)
{
	lock_acquire(&kernel_pool.lock);
	void *vaddr = malloc_page(PF_KERNEL, pg_cnt);
	if( vaddr != NULL )
		memset(vaddr, 0, pg_cnt * PAGE_SIZE);
	lock_release(&kernel_pool.lock);
	return vaddr;
}

/**
 *	用户内存分配接口函数
 *
 */
void *get_user_pages(uint32_t pg_cnt)
{
	lock_acquire(&user_pool.lock);
	void *vaddr = malloc_page(PF_USER, pg_cnt);
	if ( vaddr != NULL )
		memset(vaddr, 0, PAGE_SIZE * pg_cnt);
	lock_release(&user_pool.lock);
	return vaddr;
}


/**
 *	在相应的物理内存池pf中得到一页物理内存
 *		这一页物理内存的虚拟地址是vaddr
 */
void *get_a_page(enum pool_flags pf, uint32_t vaddr)
{
	struct pool *mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;
	lock_acquire(&mem_pool->lock);

	struct task_struct *cur = running_thread();
	int32_t bit_idx = -1;

	if ( cur->pgdir != NULL && pf == PF_USER ) {
		bit_idx = (vaddr - cur->userprog_vaddr.vaddr_start) / PAGE_SIZE;
		ASSERT(bit_idx > 0);
		bitmap_set(&cur->userprog_vaddr.vaddr_bitmap, bit_idx, 1);
	}
	else if ( cur->pgdir == NULL && pf == PF_KERNEL ) {
		bit_idx = (vaddr - kernel_vaddr.vaddr_start) / PAGE_SIZE;
		ASSERT(bit_idx > 0);
		bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx, 1);
	}
	else {
		PANIC("get_a_page: not allow kernel or user pages");
	}

	void *page_phyaddr = palloc(mem_pool);
	if ( page_phyaddr == NULL ) {
		return NULL;
	}
	page_table_add((void *)vaddr, page_phyaddr);
	lock_release(&mem_pool->lock);
	return (void *)vaddr;
}

/**
 *	将虚拟地址vaddr映射到的物理地址返回
 */
uint32_t addr_v2p(uint32_t vaddr)
{
	uint32_t *pte = pte_ptr(vaddr);
	return ((*pte & 0xfffff000) + (vaddr & 0x00000fff));
}


void *get_a_page_without_opvaddrbitmap(enum pool_flags pf, uint32_t vaddr)
{
	struct pool *mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;
	lock_acquire(&mem_pool->lock);
	void *page_phyaddr = palloc(mem_pool);
	if( page_phyaddr == NULL ) {
		lock_release(&mem_pool->lock);
		return NULL;
	}
	page_table_add((void *)vaddr, page_phyaddr);
	lock_release(&mem_pool->lock);
	return (void *)vaddr;
}





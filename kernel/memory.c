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

#define PAGE_SIZE 4096
#define MEM_BITMAP_BASE 0xc009a000
#define K_HEAP_START 0xc0100000

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

void mem_init()
{
	put_str("mem_init start\n");
	uint32_t mem_bytes_total = (*(uint32_t *)(0xb00));
	mem_pool_init(mem_bytes_total);
	put_str("mem_init done\n");
}
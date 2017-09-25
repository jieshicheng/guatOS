#include "syscall-init.h"
#include "thread.h"
#include "syscall.h"
#include "print.h"
#include "stdint.h"
#include "console.h"
#include "string.h"
#include "memory.h"
#include "fs.h"


#define syscall_nr 32

typedef void *syscall;
syscall syscall_table[syscall_nr];

uint32_t sys_getpid(void)
{
	return running_thread()->pid;
}

void syscall_init(void)
{
	put_str("syscall init start\n");
	syscall_table[SYS_GETPID] = sys_getpid;
	syscall_table[SYS_WRITE] = sys_write;
	syscall_table[SYS_MALLOC] = sys_malloc;
	syscall_table[SYS_FREE] = sys_free;
	//.....others syscall
	//.....wait declare

	put_str("syscall init done\n");
}

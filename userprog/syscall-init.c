#include "syscall-init.h"
#include "thread.h"


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
	//.....others syscall
	//.....wait declare

	put_str("syscall init done\n");
}
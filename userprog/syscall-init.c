#include "syscall-init.h"
#include "thread.h"
#include "syscall.h"
#include "print.h"
#include "stdint.h"
#include "console.h"
#include "string.h"


#define syscall_nr 32
typedef void *syscall;
syscall syscall_table[syscall_nr];

uint32_t sys_getpid(void)
{
	return running_thread()->pid;
}

uint32_t sys_write(char *str)
{
	console_put_str(str);
	return strlen(str);
}

void syscall_init(void)
{
	put_str("syscall init start\n");
	syscall_table[SYS_GETPID] = sys_getpid;
	syscall_table[SYS_WRITE] = sys_write;
	//.....others syscall
	//.....wait declare

	put_str("syscall init done\n");
}
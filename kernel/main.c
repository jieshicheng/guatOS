/**
 *  操作系统内核执行部分
 *  main入口
 *
 *  已实现功能：
 *      分段分页面机制--------在loader.s中
 *      虚拟内存管理----------在memory.c中
 *      中断机制处理----------在interrupte.c中
 *      线程调度机制----------在thread文件中
 */


#include "print.h"
#include "init.h"
#include "debug.h"
#include "thread.h"
#include "memory.h"
#include "interrupt.h"
#include "sync.h"
#include "console.h"
#include "tss.h"
#include "process.h"
#include "syscall.h"
#include "stdio.h"
#include "fs.h"
#include "shell.h"
#include "stdio-kernel.h"

int main(void)
{
    cls_screen();
    put_str("          Welcome using\n");
    put_str("          This is tiny operator system by CJS\n");
    
    init_all();
    //sys_open("/file1", O_CREAT);  OK
    //int fd = sys_open("/file1", O_WRONLY); OK
    //int fd = sys_open("/file1", O_RDONLY); OK
    //sys_read(fd, buf, 11); OK
    //sys_write(fd, "chengjieshi", 11); OK
    //sys_close(fd); OK
    //sys_unlink("/file1"); OK
    //sys_mkdir("/direct1/"); OK
    //sys_rmdir("/direct1/"); OK

    cls_screen();
    console_put_str("[rabbit@localhost /]$ ");

    while(1)
        ;
    return 0;
}


void init(void)
{
    uint32_t ret_pid = fork();
    if( ret_pid ) {
	   while(1);
    }
    else {
        my_shell();
    }
    while(1);
}



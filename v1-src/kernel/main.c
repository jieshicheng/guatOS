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
#include "ide.h"

extern struct ide_channel channels[2];

int main(void)
{
    cls_screen();
    put_str("          Welcome using\n");
    put_str("          This is tiny operator system by CJS\n");
    
    init_all();
    //int fd = sys_open("/file1", O_WRONLY); OK
    //int fd = sys_open("/file1", O_RDONLY); OK
    //sys_read(fd, buf, 11); OK
    //sys_unlink("/file1"); OK
    //sys_mkdir("/direct1/"); OK
    //sys_rmdir("/direct1/"); OK

//********************
    sys_unlink("/cat");
    uint32_t file_size = 5804;
    uint32_t sec_cnt = DIV_ROUND_UP(file_size, 512);
    struct disk *sda = &channels[0].devices[0];
    void *prog_buf = sys_malloc(file_size);
    ide_read(sda, 300, prog_buf, sec_cnt);
    int32_t fd = sys_open("/cat", O_CREAT | O_RDWR);
    if( fd != -1 ) {
        if( sys_write(fd, prog_buf, file_size) == -1 ) {
            printk("file write failed!\n");
            while(true);
        }
    }
    sys_close(fd);
    sys_free(prog_buf);


//*********************
    cls_screen();
    console_put_str("[rabbit@localhost /]$ ");
    thread_exit(running_thread(), true);
    return 0;
}


void init(void)
{
    uint32_t ret_pid = fork();
    if( ret_pid ) {
    	int status;
	int child_pid;
	while( 1 ) {
	    child_pid = wait(&status);
	    printf("i am init, my pid is 1. i recive a child %d, status is %d", child_pid, status);
	}
    }
    else {
        my_shell();
    }
}



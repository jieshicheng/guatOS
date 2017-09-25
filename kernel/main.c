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

void k_thread_a(void *arg);
void k_thread_b(void *arg);
void u_prog_a(void);
void u_prog_b(void);

int main(void)
{
    put_str("          Welcome using\n");
    put_str("          This is tiny operator system by CJS\n");
    
    init_all();
    /*
    intr_enable();
    thread_start("k_thread_a", 31, k_thread_a, "kernel thread a, my pid is: ");
    thread_start("k_thread_b", 31, k_thread_b, "kernel thread b, my pid is: ");
    process_execute(u_prog_a, "u_prog_a");
    process_execute(u_prog_b, "u_prog_b");
    intr_enable();
    */
    uint32_t fd = sys_open("/file2", O_RDWR);
    //sys_write(fd, "hello,world\n", 12);
    char buf[32] = {0};
    sys_read(fd, (void *)buf, 32);
    sys_close(fd);
    console_put_str(buf);
    while(1)
        ;
    return 0;
}

void k_thread_a(void *arg)
{
    char *msg = arg;
    void *addr1;
    void *addr2;
    void *addr3;
    void *addr4;
    void *addr5;
    void *addr6;

    uint32_t size = 16;
    addr1 = sys_malloc(size);
    size *= 2;
    addr2 = sys_malloc(size);
    size *= 2;
    addr3 = sys_malloc(size);
    size *= 2;
    addr4 = sys_malloc(size);
    size *= 2;
    addr5 = sys_malloc(size);
    size *= 2;
    addr6 = sys_malloc(size);
    sys_free(addr1);
    sys_free(addr2);
    sys_free(addr3);
    sys_free(addr4);
    sys_free(addr5);
    sys_free(addr6);

    while(1) {
        console_put_str(msg);
        console_put_char('\n');
    }
}

void k_thread_b(void *arg)
{
    char *msg = arg;
    void *addr1;
    void *addr2;
    void *addr3;
    void *addr4;
    void *addr5;
    void *addr6;

    uint32_t size = 16;
    addr1 = sys_malloc(size);
    size *= 2;
    addr2 = sys_malloc(size);
    size *= 2;
    addr3 = sys_malloc(size);
    size *= 2;
    addr4 = sys_malloc(size);
    size *= 2;
    addr5 = sys_malloc(size);
    size *= 2;
    addr6 = sys_malloc(size);

    sys_free(addr1);
    sys_free(addr2);
    sys_free(addr3);
    sys_free(addr4);
    sys_free(addr5);
    sys_free(addr6);


    while(1) {
        console_put_str(msg);
        console_put_char('\n');
    }
}

void u_prog_a(void)
{
    char *str_ua = "u_prog_a thread, my pid is: ";
    pid_t pid_ua = getpid();
    void *addr = malloc(1024);
    free(addr);
    while(1) {
        printf("%s%d\n", str_ua, pid_ua);
    }

}

void u_prog_b(void)
{
    char *str_ub = "u_prog_b thread, my pid is: ";
    pid_t pid_ub = getpid();
    void *addr = malloc(1024);
    free(addr);
    while(1) {
        printf("%s%d\n", str_ub, pid_ub);
    }
}

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

void k_thread_a(void *arg);
void k_thread_b(void *arg);

int main(void)
{
    put_str("          Welcome using\n");
    put_str("          This is tiny operator system by CJS\n");
    
    init_all();

 //   thread_start("k_thread_a", 31, k_thread_a, "argA ");
 //   thread_start("k_thread_b", 31, k_thread_b, "argB ");

    intr_enable();

    while(1);// {
       // console_put_str("main ");
   // }
    return 0;
}

void k_thread_a(void *arg)
{
    char *msg = arg;
    while(1) {
        console_put_str(arg);
    }
}

void k_thread_b(void *arg)
{
    char *msg = arg;
    while(1) {
        console_put_str(arg);
    }
}

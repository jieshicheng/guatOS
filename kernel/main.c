#include "print.h"
#include "init.h"
#include "debug.h"
#include "thread.h"
#include "memory.h"

/*
void k_thread_a(void *arg)
{
	char *msg = arg;
	while(1)
		put_str(msg);
}
*/

int main(void)
{
    put_str("          Welcome using\n");
    put_str("          This is tiny operator system by CJS\n");
    
    init_all();
    void *addr = get_kernel_pages(3);
    put_str("address start at: ");
    put_int((uint32_t)addr);
    put_char('\n');

  //  thread_start("k_thread_a", 31, k_thread_a, "argA");

    while(1);
    return 0;
}

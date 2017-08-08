#include "print.h"
#include "init.h"
#include "debug.h"

#include "memory.h"

int main(void)
{
    put_str("          Welcome using\n");
    put_str("          This is tiny operator system by CJS\n");
    
    init_all();
    void *addr = get_kernel_pages(3);
    put_str("address start at: ");
    put_int((uint32_t)addr);
    put_char('\n');
    while(1);
    return 0;
}

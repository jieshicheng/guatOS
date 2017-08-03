#include "print.h"
#include "init.h"

void main(void)
{
    put_str("          Welcome using\n")
    put_str("          This is tiny operator system by CJS\n");
    
    init_all();
    asm volatile ("sti");
    while(1);
}

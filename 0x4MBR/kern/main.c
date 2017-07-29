#include "print.h"
#include "init.h"

void main(void)
{
    put_char('K');
    put_char('E');
    put_char('R');
    put_char('N');
    put_char('E');
    put_char('L');
    put_char('\n');
    put_str("This is tiny operator system by CJS\n");
    
    init_all();
    asm volatile ("sti");
    while(1);
}

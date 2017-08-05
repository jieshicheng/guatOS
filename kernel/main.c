#include "print.h"
#include "init.h"
#include "debug.h"

int main(void)
{
    put_str("          Welcome using\n");
    put_str("          This is tiny operator system by CJS\n");
    
    init_all();
    ASSERT(1 == 2);
    while(1);
    return 0;
}

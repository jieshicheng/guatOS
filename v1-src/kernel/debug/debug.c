/**
 *	断言函数。
 *	如果发生条件不符合，则死循环暂停在此处
 */


#include "debug.h"
#include "print.h"
#include "interrupt.h"

void panic_spin(char *filename, int line, const char *func, const char *condition)
{
	intr_disable();
	put_str("!!!!!!  ERROR  !!!!!!\n");
	put_str("filename : "); put_str((char *)filename); put_str("\n");
	put_str("line : 0x"); put_int(line); put_str("\n");
	put_str("function : "); put_str((char *)func); put_str("\n");
	put_str("condition : "); put_str((char *)condition); put_str("\n");
	while(1);
}

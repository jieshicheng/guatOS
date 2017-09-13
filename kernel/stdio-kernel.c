#include "stdio.h"
#include "console.h"


/**
 *	format output by kernel using
 */
void printk(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	char buf[1024] = {0};
	vsprintf(buf, format, args);
	va_end(args);
	console_put_str(buf);
}

/**
 *	format output str to buf
 */
uint32_t sprintk(char *buf, const char *format, ...)
{
	va_list ap;
	uint32_t revalue;
	va_start(ap, format);
	revalue = vsprintf(buf, format, ap);
	va_end(ap);
	return revalue;
}

#include "stdio.h"
#include "global.h"
#include "stdint.h"
#include "string.h"
#include "syscall.h"

#define va_start(ap, v) (ap = (va_list)&v)
#define va_arg(ap, t) (*((t *)(ap += 4)))
#define va_end(ap) (ap = NULL)

static void itoa(uint32_t value, char **buf_ptr_addr, uint8_t base)
{
	uint32_t m = value % base;
	uint32_t i = value / base;
	if (i) {
		itoa(i, buf_ptr_addr, base);
	}
	if (m < 10) {
		*((*buf_ptr_addr)++) = m + '0';
	}
	else {
		*((*buf_ptr_addr)++) = m - 10 + 'A';
	}
}


uint32_t vsprintf(char *str, const char *format, va_list ap)
{
	char *buf_ptr = str;
	const char *index_ptr = format;
	char index_char = *index_ptr;
	int32_t arg_int;
	while (index_char) {
		if (index_char != '%') {
			*(buf_ptr++) = index_char;
			index_char = *(++index_ptr);
			continue;
		}
		index_char = *(++index_ptr);
		switch (index_char) {
			case 'x':
				arg_int = va_arg(ap, int);
				itoa(arg_int, &buf_ptr, 16);
				index_char = *(++index_ptr);
				break;
		}
	}
	return strlen(str);
}

uint32_t printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	char buf[1024] = {0};
	vsprintf(buf, format, args);
	va_end(args);
	return write(buf);
}

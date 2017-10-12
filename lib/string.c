/**
 *	字符串操作函数
 *
 */

#include "string.h"
#include "debug.h"
#include "global.h"
#include "stdint.h"

// 按字节填充内存函数
void memset(void *dst_, uint8_t value, uint32_t size)
{
	ASSERT(dst_ != NULL);
	uint8_t *dst = (uint8_t *)dst_;
	while(size-- > 0) {
		*dst++ = value;
	}
}

// 按字节拷贝函数
void memcpy(void *dst_, const void *src_, uint32_t size)
{
	ASSERT(dst_ != NULL && src_ != NULL);
	uint8_t *dst = (uint8_t *)dst_;
	const uint8_t *src = (const uint8_t *)src_;
	while(size-- > 0) {
		*dst++ = *src++;
	}
}

// 按字节比较函数
int memcmp(const void *a_, const void *b_, uint32_t size)
{
	const char *a = a_;
	const char *b = b_;
	ASSERT(a != NULL || b != NULL);
	while(size-- > 0) {
		if(*a != *b) {
			return *a > *b ? 1 : -1;
		}
		a++;
		b++;
	}
	return 0;
}

// 字符串复制函数
char *strcpy(char *dst_, const char *src_)
{
	ASSERT(dst_ != NULL && src_ != NULL);
	char *r = dst_;
	while((*dst_++ = *src_++));
	return r;
}

// 返回字符串长度函数
uint32_t strlen(const char *str)
{
	ASSERT(str != NULL);
	const char *p = str;
	while(*p++);
	return (p - str - 1);
}

// 字符串比较函数
int8_t strcmp(const char *a, const char *b)
{
	ASSERT(a != NULL && b != NULL);
	while(*a != 0 && *a == *b) {
		a++;
		b++;
	}

	return *a < *b ? -1 : *a > *b;
}

// 字符串中查找给定字符第一个位置函数
char *strchr(const char *str, const uint8_t ch) 
{
	ASSERT(str != NULL);
	while(*str != 0) {
		if(*str == ch)
			return (char *)str;
		str++;
	}
	return NULL;
}

// 字符串中查找给定字符的最后一个位置函数
char *strrchr(const char *str, const uint8_t ch)
{
	ASSERT(str != NULL);
	char *last_curr = NULL;
	while(*str != 0) {
		if(*str == ch)
			last_curr = (char *)str;
		str++;
	}
	return last_curr;

}

// 两个字符串拼接函数
char *strcat(char *dst_, const char *src_)
{
	ASSERT(dst_ != NULL && src_ != NULL);
	char *str = dst_;
	while(*str++);
	--str;
	while(*src_) {
		*str++ = *src_++;
	}
	return dst_;
}

// 字符串中查找给定字符的出现次数函数
uint32_t strchrs(const char *str, uint8_t ch)
{
	ASSERT(str != NULL);
	uint32_t ch_cnt = 0;
	const char *p = str;
	while(*p != 0) {
		if(*p == ch)
			ch_cnt++;
		p++;
	}
	return ch_cnt;
}

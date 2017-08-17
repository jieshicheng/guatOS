/**
 *	位图定义。用来实现资源管理
 *
 */



#include "bitmap.h"
#include "stdint.h"
#include "string.h"
#include "print.h"
#include "interrupt.h"
#include "debug.h"

void bitmap_init(struct bitmap *btmp)
{
	memset(btmp->bits, 0, btmp->btmp_bytes_len);
}

int bitmap_scan_test(struct bitmap *btmp, uint32_t bit_idx)
{
	uint32_t byte_idx = bit_idx / 8;
	uint32_t byte_odd = bit_idx % 8;
	return (btmp->bits[byte_idx] & (BITMAP_MASK << byte_odd));
}


int bitmap_scan(struct bitmap *btmp, uint32_t cnt)
{
	uint32_t byte_index = 0;
	while(btmp->bits[byte_index] == 0xff && byte_index < btmp->btmp_bytes_len) {
		++byte_index;
	}
	ASSERT(byte_index < btmp->btmp_bytes_len);
	if(byte_index == btmp->btmp_bytes_len) {
		return -1;
	}
	int bit_index = 0;
	while(btmp->bits[byte_index] & (BITMAP_MASK << bit_index)) {
		bit_index++;
	}
	uint32_t bit_start = byte_index * 8 + bit_index;
	if(cnt == 1) {
		return bit_start;
	}

	uint32_t bit_left = btmp->btmp_bytes_len * 8 - bit_start;
	uint32_t bit_next = bit_start + 1;
	uint32_t count = 1;
	bit_start = -1;
	while(bit_left-- > 0) {
		if( !bitmap_scan_test(btmp, bit_next) ) {
			count++;
		}
		else {
			count = 0;
		}
		if(count == cnt) {
			bit_start = bit_next - cnt + 1;
			break;
		}
		bit_next++;
	}
	return bit_start;
}

void bitmap_set(struct bitmap *btmp, uint32_t bit_idx, int8_t value)
{
	ASSERT(value == 0 || value == 1);
	uint32_t byte_index = bit_idx / 8;
	uint32_t bit_index = bit_idx % 8;

	if(value) {
		btmp->bits[byte_index] |= (BITMAP_MASK << bit_index);
	}
	else {
		btmp->bits[byte_index] &= ~(BITMAP_MASK << bit_index);
	}
}


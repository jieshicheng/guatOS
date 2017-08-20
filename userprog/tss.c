#include "stdint.h"
#include "global.h"
#include "print.h"
#include "string.h"
#include "tss.h"

static struct tss tss;

void update_tss_esp(struct task_struct *pthread)
{
	tss.esp0 = (uint32_t *)((uint32_t)pthread + PAGE_SIZE);
}

static struct gdt_desc make_gdt_desc(uint32_t *desc_addr, uint32_t limit, uint8_t attr_low, uint8_t attr_high)
{
	uint32_t desc_base = (uint32_t)desc_addr;
	struct gdt_desc desc;
	desc.limit_low_word = limit & 0x0000ffff;
	desc.base_low_word = desc_base & 0x0000ffff;
	desc.base_mid_byte = ((desc_base & 0x00ff0000) >> 16);
	desc.attr_low_byte = (uint8_t)attr_low;
	desc.limit_high_attr_high = (((limit & 0x000f0000) >> 16) + (uint8_t)(attr_high));
	desc.base_high_byte = desc_base >> 24;
	return desc;
}

void tss_init()
{
	put_str("tss_init start\n");
	uint32_t tss_size = sizeof(tss);
	memset(&tss, 0, tss_size);
	tss.ss0 = SELECTOR_K_STACK;
	tss.io_base = tss_size;

	*((struct gdt_desc *)0xc0000923) = make_gdt_desc((uint32_t *)&tss, tss_size - 1, TSS_ATTR_LOW, TSS_ATTR_HIGH);
	*((struct gdt_desc *)0xc000092b) = make_gdt_desc((uint32_t *)0, 0xfffff, GDT_CODE_ATTR_LOW_DPL3, GDT_ATTR_HIGH);
	*((struct gdt_desc *)0xc0000933) = make_gdt_desc((uint32_t *)0, 0xfffff, GDT_DATA_ATTR_LOW_DPL3, GDT_ATTR_HIGH);

	uint64_t gdt_operand = 0x0000c00009030037;//

	asm volatile ("lgdt %0" : : "m"(gdt_operand));
	asm volatile ("ltr %w0" : : "r"(SELECTOR_TSS));
	
	put_str("tss_init done\n");
}

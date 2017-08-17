#include "stdint.h"
#include "global.h"
#include "io.h"
#include "print.h"
#include "interrupt.h"


struct gate_desc {
	uint16_t	func_offset_low_word;
	uint16_t	selector;
	uint8_t		dcount;
	uint8_t		attribute;
	uint16_t	func_offset_high_word;

};

char *intr_name[IDT_DESC_CNT];
intr_handler idt_table[IDT_DESC_CNT];
static struct gate_desc idt[IDT_DESC_CNT];


extern intr_handler intr_entry_table[IDT_DESC_CNT];


enum intr_status intr_enable()
{
	enum intr_status old_status;
	if(INTR_ON == intr_get_status()) {
		old_status = INTR_ON;
		return old_status;
	}
	else {
		old_status = INTR_OFF;
		asm volatile("sti");
		return old_status;
	}
}

enum intr_status intr_disable()
{
	enum intr_status old_status;
	if(INTR_ON == intr_get_status()) {
		old_status = INTR_ON;
		asm volatile("cli" : : : "memory");
		return old_status;
	}
	else {
		old_status = INTR_OFF;
		return old_status;
	}
}

enum intr_status intr_set_status(enum intr_status status)
{
	return status & INTR_ON ? intr_enable() : intr_disable();
}

enum intr_status intr_get_status()
{
	uint32_t eflags = 0;
	GET_EFLAGS(eflags);
	return (eflags & EFLAGS_IF) ? INTR_ON : INTR_OFF;
}

static void general_intr_handler(uint8_t vec_nr)
{
	if(vec_nr == 0x27 || vec_nr == 0x2f)
		return ;
	set_cursor(0);  //***********
	int cursor_pos = 0;
	while ( cursor_pos < 320 ) {
		put_char(' ');
		cursor_pos++;
	}
	set_cursor(0);
	put_str("!!!!!   exception message begin  !!!!!!\n");
	set_cursor(88);
	put_str(intr_name[vec_nr]);
	if ( vec_nr == 14 ) {
		int page_fault_vaddr = 0;
		asm volatile ("movl %%cr2, %0" : "=r"(page_fault_vaddr));
		put_str("\npage fault addr is: "); 
		put_int(page_fault_vaddr);
	}
	put_str("\n!!!!!   exception message end   !!!!!!\n");
	while (1);
}

void register_handler(uint8_t vector_no, intr_handler function)
{
	idt_table[vector_no] = function;
}

static void exception_init(void)
{
	int i;
	for(i = 0; i != IDT_DESC_CNT; ++i)
	{
		idt_table[i] = general_intr_handler;
		intr_name[i] = "unknow";
	}
	intr_name[0] = "#DE Divide Error";
	intr_name[1] = "#DB Debug Exception";
	intr_name[2] = "#INT Interrupt";
	intr_name[3] = "#BP BreakPoint Exception";
	intr_name[4] = "#OF OverFlow Exception";
	intr_name[5] = "#BR Bound Range Exceeded Exception";
	intr_name[6] = "#UD Invalid Opcode Exception";
	intr_name[7] = "#NM Device Not Available Exception";
	intr_name[8] = "#DF Double fault Exception";
	intr_name[9] = "#Coprocessor Segment Overrun";
	intr_name[10] = "#TS Invalid TSS Exception";
	intr_name[11] = "#NP Segment Not Present";
	intr_name[12] = "#SS Stack fault Exception";
	intr_name[13] = "#GP General Protect Exception";
	intr_name[14] = "#PF Page_fault Exception";
	intr_name[16] = "#MF x86 FPU Floating-Point Error";
	intr_name[17] = "#AC Alignment Check Exception";
	intr_name[18] = "#MC Machine-Check Exception";
	intr_name[19] = "#XF SIMD Floating-Point Exception";

}

static void pic_init(void)
{
	outb(PIC_M_CTRL, 0x11);
	outb(PIC_M_DATA, 0x20);
	outb(PIC_M_DATA, 0x04);
	outb(PIC_M_DATA, 0x01);

	outb(PIC_S_CTRL, 0x11);
	outb(PIC_S_DATA, 0x28);
	outb(PIC_S_DATA, 0x02);
	outb(PIC_S_DATA, 0x01);

	outb(PIC_M_DATA, 0xfc);
	outb(PIC_S_DATA, 0xff);

	put_str("	pic_init done\n");
}

static void make_idt_desc(struct gate_desc *p_gdesc, uint8_t attr, intr_handler function)
{
	p_gdesc->func_offset_low_word = (uint32_t)function & 0x0000ffff;
	p_gdesc->selector = SELECTOR_K_CODE;
	p_gdesc->dcount = 0;
	p_gdesc->attribute = attr;
	p_gdesc->func_offset_high_word = ((uint32_t)function & 0xffff0000) >> 16;

}

static void idt_desc_init(void)
{
	int i;
	for(i = 0; i != IDT_DESC_CNT; ++i) {
		make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]);
	}
	put_str("	idt_desc_init done\n");

}

void idt_init() 
{
	put_str("idt_init start\n");
	idt_desc_init();
	exception_init();
	pic_init();

	uint64_t idt_operand = ((sizeof(idt) - 1) | ((uint64_t)((uint32_t)idt << 16)));
	asm volatile ("lidt %0" : : "m" (idt_operand));
	put_str("idt_init done\n");
}




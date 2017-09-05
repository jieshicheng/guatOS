/**
 *	与中断处理机制相关的函数都在此文件中
 *
 *
 *
 */

#include "stdint.h"
#include "global.h"
#include "io.h"
#include "print.h"
#include "interrupt.h"

// 中断们描述符结构体
struct gate_desc {
	uint16_t	func_offset_low_word;
	uint16_t	selector;
	uint8_t		dcount;
	uint8_t		attribute;
	uint16_t	func_offset_high_word;

};

// 各个中断名称
char *intr_name[IDT_DESC_CNT];
// 各个中断对应的处理函数表
intr_handler idt_table[IDT_DESC_CNT];
// 定义中断表
static struct gate_desc idt[IDT_DESC_CNT];

// 在kernel.s中定义的中断处理例程入口
extern intr_handler intr_entry_table[IDT_DESC_CNT];

// 打开中断
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

// 关闭中断
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

// 设置中断状态根据参数
enum intr_status intr_set_status(enum intr_status status)
{
	return status & INTR_ON ? intr_enable() : intr_disable();
}

// 得到当前中断状态
enum intr_status intr_get_status()
{
	uint32_t eflags = 0;
	GET_EFLAGS(eflags);
	return (eflags & EFLAGS_IF) ? INTR_ON : INTR_OFF;
}

// 统一的中断处理函数。
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

// 安装中断处理函数
void register_handler(uint8_t vector_no, intr_handler function)
{
	idt_table[vector_no] = function;
}

// 中断入口初始化， 初始化名字， 对应的处理函数
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

// 初始化可编程中断控制器
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

	outb(PIC_M_DATA, 0xfc); // 开关外部中断的位。0xfc表示倒数的时钟，键盘中断打开
	outb(PIC_S_DATA, 0xff);

	put_str("	pic_init done\n");
}

// 设置中断描述符
static void make_idt_desc(struct gate_desc *p_gdesc, uint8_t attr, intr_handler function)
{
	p_gdesc->func_offset_low_word = (uint32_t)function & 0x0000ffff;
	p_gdesc->selector = SELECTOR_K_CODE;
	p_gdesc->dcount = 0;
	p_gdesc->attribute = attr;
	p_gdesc->func_offset_high_word = ((uint32_t)function & 0xffff0000) >> 16;

}

// 中断描述符初始化
static void idt_desc_init(void)
{
	int i;
	for(i = 0; i != IDT_DESC_CNT; ++i) {
		make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]);
	}
	put_str("	idt_desc_init done\n");

}

/**
 *	总的初始化
 *		先初始所有的中断描述符
 *		再设置各个中断的转发函数以及中断名称
 *		最后初始化8259可编程中断控制器。将外部设备的中断口打开
 */
void idt_init() 
{
	put_str("idt_init start\n");
	idt_desc_init();
	exception_init();
	pic_init();

	uint64_t idt_operand = ((sizeof(idt) - 1) | ((uint64_t)(uint32_t)idt << 16));
	asm volatile ("lidt %0" : : "m" (idt_operand));
	put_str("idt_init done\n");
}




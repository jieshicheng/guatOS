#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H

#include "stdint.h"

#define IDT_DESC_CNT 0x21
#define PIC_M_CTRL 0x20
#define PIC_M_DATA 0x21
#define PIC_S_CTRL 0xa0
#define PIC_S_DATA 0xa1
#define EFLAGS_IF 0x00000200
#define GET_EFLAGS(EFLAGS_VAR) asm volatile("pushfl; popl %0" : "=g" (EFLAGS_VAR))


struct gate_desc;
typedef void * intr_handler;
enum intr_status { INTR_OFF, INTR_ON };


static void make_idt_desc(struct gate_desc *, uint8_t, intr_handler);
static void idt_desc_init(void);
void idt_init();
static void general_intr_handler(uint8_t vec_nr);
static void pic_init(void);
static void exception_init(void);
enum intr_status intr_enable();
enum intr_status intr_disable();
enum intr_status intr_set_status(enum intr_status status);
enum intr_status intr_get_status();


#endif

#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H

struct gate_desc;
typedef void * intr_handler;

static void make_idt_desc(struct gate_desc *, uint8_t, intr_handler);
static void idt_desc_init(void);
void idt_init();
static void general_intr_handler(uint8_t vec_nr);
static void pic_init(void);
static void exception_init(void);

#endif

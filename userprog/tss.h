#ifndef __KERNEL_TSS_H
#define __KERNEL_TSS_H

struct tss;

void update_tss_esp(struct task_struct *pthread);
static struct gdt_desc make_gdt_desc(uint32_t *desc_addr, uint32_t limit, uint8_t attr_low, uint8_t attr_high);
void tss_init();



#endif
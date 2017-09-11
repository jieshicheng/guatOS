#include "ide.h"
#include "stdint.h"
#include "stdio-kernel.h"
#include "debug.h"
#include "global.h"
#include "io.h"
#include "timer.h"
#include "sync.h"
#include "interrupt.h"

#define reg_data(channel) 		(channel->port_base + 0)
#define reg_error(channel)		(channel->port_base + 1)
#define reg_sect_cnt(channel)	(channel->port_base + 2)
#define reg_lba_l(channel)		(channel->port_base + 3)
#define reg_lba_m(channel)		(channel->port_base + 4)
#define reg_lba_h(channel)		(channel->port_base + 5)
#define reg_dev(channel)		(channel->port_base + 6)
#define reg_status(channel)		(channel->port_base + 7)
#define reg_cmd(channel)		(reg_status(channel))
#define reg_alt_status(channel)	(channel->port_base + 0x206)
#define reg_ctl(channel)		(reg_alt_status(channel))


#define BIT_ALT_STAT_BSY	0x80
#define BIT_ALT_STAT_DRDY	0x40
#define BIT_ALT_STAT_DRQ	0x8
#define BIT_DEV_MBS			0xa0
#define BIT_DEV_LBA			0x40
#define BIT_DEV_DEV			0x10

#define CMD_IDENTIFY		0xec
#define CMD_READ_SECTOR		0x20
#define CMD_WRITE_SECTOR	0x30

#define max_lba 			((80 * 1024 * 1024 / 512) - 1)

uint8_t channel_cnt;
struct ide_channel channels[2];

void ide_init()
{
	printk("ide_init start \n");
	uint8_t hd_cnt = *((uint8_t *)(0x475));
	ASSERT(hd_cnt > 0);
	channel_cnt = DIV_ROUND_UP(hd_cnt, 2);

	struct ide_channel *channel;
	uint8_t channel_no = 0;
	while (channel_no < channel_cnt) {
		channel = &channels[channel_no];

		sprintf(channel->name, "ide%d", channel_no);
		switch (channel_no) {
			case 0:
				channel->port_base = 0x1f0;
				channel->irq_no = 0x20 + 14;
				break;
			case 1:
				channel->port_base = 0x170;
				channel->irq_no = 0x20 + 15;
				break;
		}
		channel->expecting_intr = false;
		lock_init(&channel->lock);
		sema_init(&channel->disk_done, 0);
		register_handler(channel->irq_no, intr_hd_handler);
		channel_no++;
	}
	printk("ide_init done\n");
}


static void select_disk(struct disk *hd)
{
	uint8_t reg_device = (BIT_DEV_MBS | BIT_DEV_LBA);
	if (hd->dev_no == 1) {
		reg_device |= BIT_DEV_DEV;
	}
	outb(reg_dev(hd->my_channel), reg_device);
}

static void select_sector(struct disk *hd, uint32_t lba, uint8_t sec_cnt)
{
	ASSERT(lba <= max_lba);
	struct ide_channel *channel = hd->my_channel;
	
	outb(reg_sect_cnt(channel), sec_cnt);
	outb(reg_lba_l(channel), lba);
	outb(reg_lba_m(channel), lba >> 8);
	outb(reg_lba_h(channel), lba >> 16);
	outb(reg_dev(channel), (BIT_DEV_MBS | 
							BIT_DEV_LBA | 
							(hd->dev_no == 1 ? BIT_DEV_DEV : 0) | 
							lba >> 24));
}

static void cmd_out(struct ide_channel *channel, uint8_t cmd)
{
	channel->expecting_intr = true;
	outb(reg_cmd(channel), cmd);
}

static void read_from_sector(struct disk *hd, void *buf, uint8_t sec_cnt)
{
	uint32_t size_in_byte;
	if (sec_cnt == 0) {
		size_in_byte = 256 * 512;
	}
	else {
		size_in_byte = sec_cnt * 512;
	}
	insw(reg_data(hd->channel), buf, size_in_byte / 2);
}

static void write2sector(struct disk *hd, void *buf, uint8_t sec_cnt)
{
	uint32_t size_out_byte;
	if (sec_cnt == 0) {
		size_out_byte = 256 * 512;
	}
	else {
		size_out_byte = sec_cnt * 512;
	}
	outsw(reg_data(hd->channel), buf, size_out_byte / 2);
}

static enum bool busy_wait(struct disk *hd)
{
	struct ide_channel *channel = hd->my_channel;
	uint16_t time_limit = 30 * 1000;
	while ((time_limit -= 10) >= 0) {
		if (!(inb(reg_status(channel)) & BIT_ALT_STAT_BSY)) {
			return (inb(reg_status(channel)) & BIT_ALT_STAT_DRQ);
		}
		else {
			mtime_sleep(10);
		}
	}
	return false;
}


void ide_read(struct disk *hd, uint32_t lba, void *buf, uint32_t sec_cnt)
{
	ASSERT(lba <= max_lba);
	ASSERT(sec_cnt > 0);
	lock_acquire(&hd->my_channel->lock);

	select_disk(hd);

	uint32_t secs_op;
	uint32_t secs_done = 0;

	while (secs_done <= sec_cnt) {
		if ((secs_done + 256) <= sec_cnt) {
			secs_op = 256;
		}
		else {
			secs_op = sec_cnt - secs_done;
		}

		select_sector(hd, lba + secs_done, secs_op);
		cmd_out(hd->my_channel, CMD_READ_SECTOR);

		sema_down(&hd->my_channel->disk_done);

		if (!busy_wait(hd)) {
			char error[64];
			sprintf(error, "%s read sector %d failed !!!!!\n", hd->name, lba);
			PANIC(error);
		}

		read_from_sector(hd, (void *)((uint32_t)buf + secs_done * 512), secs_op);
		secs_done += secs_op;
	}
	lock_release(&hd->my_channel->disk_done);
}

void ide_write(struct disk *hd, uint32_t lbd, void *buf, uint32_t sec_cnt)
{
	ASSERT(lba <= max_lba);
	ASSERT(sec_cnt > 0);
	lock_acquire(&hd->my_channel->lock);

	select_disk(hd);

	uint32_t secs_op;
	uint32_t secs_done = 0;
	while (secs_done < sec_cnt) {
		if ((secs_done + 256) <= sec_cnt) {
			secs_op = 256;
		}
		else {
			secs_op = sec_cnt - secs_done;
		}

		select_sector(hd, lba + secs_done, secs_op);
		cmd_out(hd->my_channel, CMD_WRITE_SECTOR);

		if (!busy_wait(hd)) {
			char error[64];
			sprintf(error, "%s write sector %d failed !!!!\n", hd->name, lba);
			PANIC(error);
		}

		write2sector(hd, (void *)((uint32_t)buf + secs_done * 512), secs_op);

		sema_down(&hd->my_channel->disk_done);
		secs_done += secs_op;
	}
	lock_release(&hd->my_channel->lock);
}



void intr_hd_handler(uint8_t irq_no)
{
	ASSERT(irq_no == 0x2e || irq_no == 0x2f);
	uint8_t ch_no = irq_no - 0x2e;
	struct ide_channel *channel = &channels[ch_no];
	ASSERT(channel->irq_no == irq_no);
	if (channel->expecting_intr) {
		channel->expecting_intr = false;
		sema_up(&channel->disk_done);
		inb(reg_status(channel));
	}
}





























































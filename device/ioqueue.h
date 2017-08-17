#ifndef __KERNEL_IOQUEUE_H
#define __KERNEL_IOQUEUE_H

#include "stdint.h"
#include "thread.h"
#include "sync.h"

#define bufsize 64

struct ioqueue
{
	struct lock lock;
	struct task_struct *producer;
	struct task_struct *consumer;
	char buf[bufsize];
	int32_t head;
	int32_t tail;
};

void ioqueue_init(struct ioqueue *ioq);
static int32_t next_pos(int32_t pos);
enum bool ioq_full(struct ioqueue *ioq);
enum bool ioq_empty(struct ioqueue *ioq);
static void ioq_wait(struct task_struct **waiter);
static void ioq_wakeup(struct task_struct **waiter);
char ioq_getchar(struct ioqueue *ioq);
void ioq_putchar(struct ioqueue *ioq, char byte);

#endif
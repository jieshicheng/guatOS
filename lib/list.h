#ifndef __KERNEL_LIST_H
#define __KERNEL_LIST_H

#include "global.h"
#include "stdint.h"

#define offset(struct_type, member) (int)(&((struct_type *)0)->member)
#define elem2entry(struct_type, struct_member_name, ele_ptr) \
		(struct_type *)((int)ele_ptr - offset(struct_type, struct_member_name))



struct list_elem
{
	struct list_elem *prev;
	struct list_elem *next;
};

struct list
{
	struct list_elem head;
	struct list_elem tail;
};


typedef enum bool (function)(struct list_elem *, int);

void list_init(struct list *plist);
void list_insert_before(struct list_elem *before, struct list_elem *elem);
void list_push(struct list *plist, struct list_elem *elem);
//void list_iterate(struct list *plist);
void list_append(struct list *plist, struct list_elem *elem);
void list_remove(struct list_elem *elem);
struct list_elem *list_pop(struct list *plist);
enum bool list_empty(struct list *plist);
uint32_t list_len(struct list *plist);
struct list_elem *list_traversal(struct list *plist, function func, int arg);
enum bool elem_find(struct list *plist, struct list_elem *elem);


#endif

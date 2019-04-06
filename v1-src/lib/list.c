/**
 *	内核中重要的数据结构---双向链表实现
 */

#include "list.h"
#include "interrupt.h"
#include "stdint.h"
#include "global.h"

void list_init(struct list *plist)
{
	plist->head.prev = NULL;
	plist->head.next = &plist->tail;
	plist->tail.prev = &plist->head;
	plist->tail.next = NULL;
}

void list_insert_before(struct list_elem *before, struct list_elem *elem)
{
	enum intr_status old_status = intr_disable();

	before->prev->next = elem;
	elem->prev = before->prev;
	elem->next = before;
	before->prev = elem;

	intr_set_status(old_status);
}

void list_push(struct list *plist, struct list_elem *elem)
{
	list_insert_before(plist->head.next, elem);
}

//void list_iterate(struct list *plist)


void list_append(struct list *plist, struct list_elem *elem)
{
	list_insert_before(&plist->tail, elem);

}

void list_remove(struct list_elem *elem)
{
	enum intr_status old_status = intr_disable();

	elem->prev->next = elem->next;
	elem->next->prev = elem->prev;

	intr_set_status(old_status);
}
struct list_elem *list_pop(struct list *plist)
{
	struct list_elem *top = plist->head.next;
	list_remove(top);
	return top;
}

enum bool list_empty(struct list *plist)
{
	return (plist->head.next == &plist->tail ? true : false);
}

uint32_t list_len(struct list *plist)
{
	uint32_t len = 0;
	struct list_elem *point = plist->head.next;
	while(point != &plist->tail) {
		len++;
		point = point->next;
	}
	return len;
}

struct list_elem *list_traversal(struct list *plist, function func, int arg)
{
	if( list_empty(plist) )
		return NULL;
	struct list_elem *point = plist->head.next;
	while( point != &plist->tail ) {
		if( func(point, arg) )
			return point;
		else
			point = point->next;
	}
	return NULL;
}

enum bool elem_find(struct list *plist, struct list_elem *elem)
{
	struct list_elem *point = plist->head.next;
	while( point != &plist->tail ) {
		if( point == elem )
			return true;
		else
			point = point->next;
	}
	return false;
}




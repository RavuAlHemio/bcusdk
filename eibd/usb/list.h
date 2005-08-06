#ifndef _LIST_H_
#define _LIST_H_

struct list_head {
  struct list_head *prev, *next;
};

/* These implement Linux kernel style double linked lists */
void list_init(struct list_head *entry);
void list_add(struct list_head *entry, struct list_head *head);
void list_del(struct list_head *entry);

#define list_entry(ptr, type, member) \
	((type *)((char *)(ptr) - (unsigned long)(&((type *)0L)->member)))
#define list_for_each_entry(pos, head, member)				\
	for (pos = list_entry((head)->next, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = list_entry(pos->member.next, typeof(*pos), member))
#define list_for_each_entry_safe(pos, n, head, member)			\
        for (pos = list_entry((head)->next, typeof(*pos), member),	\
		n = list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))
#define list_empty(entry) ((entry)->next == (entry))

#endif /* _LIST_H_ */


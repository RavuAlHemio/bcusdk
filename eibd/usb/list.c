#include "usbi.h"

/* Some quick and generic macros for the simple kind of lists we use */
void list_init(struct list_head *entry)
{
  entry->prev = entry->next = entry;
}

void list_add(struct list_head *entry, struct list_head *head)
{
  entry->next = head;
  entry->prev = head->prev;

  head->prev->next = entry;
  head->prev = entry;
}

void list_del(struct list_head *entry)
{
  entry->next->prev = entry->prev;
  entry->prev->next = entry->next;
}


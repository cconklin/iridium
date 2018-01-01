/*
  linked_list.h

  Linked list library.
*/

#include <stdlib.h>
#include <assert.h>

#include "../garbage_collector.h"

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

struct list {
  void * head;
  struct list * tail;
};

void list_extend(struct list *, void *);
struct list * list_cons(struct list * tail, void * head);
void list_destroy(struct list * l);
unsigned int list_length(struct list * l);

// Shortcut constructor for cons with NULL
#define list_new(head) list_cons(NULL, head)

// list_head
// inputs: l (struct list *)
// returns: void *
// Returns the head of the list `l`
#define list_head(l) l -> head

// list_tail
// inputs: l (struct list *)
// returns struct list *
// Returns the tail of the list `l`
#define list_tail(l) l -> tail

#endif

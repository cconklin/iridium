/*
  linked_list.h

  Linked list library underlying the List class, among others.
*/

#include <stdlib.h>
#include <assert.h>

// HACK around GC not linking
#define GC_MALLOC(n) calloc(1, n)
#define GC_REALLOC(p, n) realloc(p, n)

// Shortcut constructor for cons with NULL
#define list_new(head) list_cons(NULL, head)

struct list {
  void * head;
  struct list * tail;
};

struct list * list_cons(struct list * tail, void * head);
void list_destroy(struct list * l);

// list_cons
// inputs: tail (struct list *), head (void *)
// returns: struct list *
// Extents list `tail` by adding an element with data `head`,
// returning the new list
struct list * list_cons(struct list * tail, void * head) {
  // Allocate a new node
  struct list * l = GC_MALLOC(sizeof(struct list));
  // Ensure that the node was created
  assert(l);
  // Set the head of the node to the data passed
  l -> head = head;
  // Set the tail of the node to the passed list
  l -> tail = tail;
  
  return l;
}

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

// list_destroy
// inputs: l (struct list *)
// returns:
// Destroy and deallocate `l`
void list_destroy(struct list * l) {
  struct list * temp;
  while (l) {
    temp = l -> tail;
    // set to NULL so the GC is able to collect the head, if needed
    l -> head = NULL;
    free(l);
    l = temp;
  }
  l = NULL;
}

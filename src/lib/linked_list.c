/*
  linked_list.c

  Linked list library.
*/

#include "linked_list.h"

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

// list_length
// inputs: l (struct list *)
// returns: length (unsigned int)
// Returns the length of the passed list
unsigned int list_length(struct list * l) {
  unsigned int length = 0;
  while (l) {
    length ++;
    l = l -> tail;
  }
  return length;
}
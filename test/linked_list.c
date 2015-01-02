#include "../src/linked_list.h"

int main (int argc, char const *argv[]) {
  struct list * l = list_new(5);
  
  // It should have a head
  assert(list_head(l) == 5);
  // It should have no tail
  assert(list_tail(l) == NULL);
  
  // cons
  // it should add a new head
  assert(list_head(list_cons(l, 7)) == 7);
  // it should make the old list the tail
  assert(list_tail(list_cons(l, 7)) == l);
  
  return 0;
}
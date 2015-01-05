#include "../src/linked_list.h"
#include "test_helper.h"

int main (int argc, char const *argv[]) {
  struct list * l = list_new(5);
  
  // It should have a head
  assertEqual(list_head(l), 5);
  // It should have no tail
  assertEqual(list_tail(l), NULL);
  
  // cons
  // it should add a new head
  assertEqual(list_head(list_cons(l, 7)), 7);
  // it should make the old list the tail
  assertEqual(list_tail(list_cons(l, 7)), l);
  
  // length
  // it should know its length
  assertEqual(list_length(l), 1);

  return 0;
}
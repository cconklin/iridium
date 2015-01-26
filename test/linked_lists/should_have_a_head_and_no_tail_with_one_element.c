#include "../../src/lib/linked_list.h"
#include "../test_helper.h"

int main (int argc, char const *argv[]) {
  struct list * l = list_new(5);

  // It should have a head
  assertEqual(list_head(l), 5);
  // It should have no tail
  assertEqual(list_tail(l), NULL);

  return 0;
}
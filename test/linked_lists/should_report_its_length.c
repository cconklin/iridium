#include "../../src/lib/linked_list.h"
#include "../test_helper.h"

int test() {
  struct list * l = list_new(5);

  assertEqual(list_length(l), 1);

  return 0;
}

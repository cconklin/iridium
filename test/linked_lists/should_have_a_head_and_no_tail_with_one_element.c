#include "../../iridium/include/shared/linked_list.h"
#include "../test_helper.h"

int test(struct IridiumContext * context) {
  struct list * l = list_new(5);

  // It should have a head
  assertEqual(list_head(l), 5);
  // It should have no tail
  assertEqual(list_tail(l), NULL);

  return 0;
}

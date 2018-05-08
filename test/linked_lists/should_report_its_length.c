#include "../../iridium/include/shared/linked_list.h"
#include "../test_helper.h"

int test(struct IridiumContext * context) {
  struct list * l = list_new(5);

  assertEqual(list_length(l), 1);

  return 0;
}

#include "../../../iridium/include/shared/linked_list.h"
#include "../../test_helper.h"

int test() {
  struct list * l = list_new(5);

  assertEqual(list_head(list_cons(l, 7)), 7);

  return 0;
}

#include "../../iridium/include/shared/array.h"
#include "../test_helper.h"

int test() {
  struct array * ary = array_new();

  array_set(ary, 0, 1);
  assertEqual(array_get(array_copy(ary), 0), 1);

  return 0;
}

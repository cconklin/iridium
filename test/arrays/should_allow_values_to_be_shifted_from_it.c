#include "../../iridium/include/shared/array.h"
#include "../test_helper.h"

int test(struct IridiumContext * context) {
  struct array * ary = array_new();

  array_set(ary, 0, 1);
  array_set(ary, 1, 2);
  
  assertEqual(array_shift(ary), 1);
  assertEqual(array_shift(ary), 2);
  assertEqual(array_get(ary, 0), NULL);

  return 0;
}

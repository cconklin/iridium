#include "../../src/lib/array.h"
#include "../test_helper.h"

int test() {
  struct array * ary = array_new();

  array_set(ary, 1, 0x05);
  assertEqual(array_get(ary, 1), 0x05);

  return 0;
}

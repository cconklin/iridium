#include "../../iridium/include/shared/array.h"
#include "../test_helper.h"

int test() {
  struct array * ary = array_new();

  array_set(ary, 100, 0x04);
  assertEqual(array_get(ary, 100), 0x04);

  return 0;
}

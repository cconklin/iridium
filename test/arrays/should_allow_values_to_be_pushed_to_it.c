#include "../../iridium/include/shared/array.h"
#include "../test_helper.h"

int test() {
  struct array * ary = array_new();

  ary = array_new();
  array_push(ary, 1);
  array_push(ary, 2);
  assertEqual(array_get(ary, 0), 1);
  assertEqual(array_get(ary, 1), 2);

  return 0;
}

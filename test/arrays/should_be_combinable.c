#include "../../iridium/include/shared/array.h"
#include "../test_helper.h"

int test(struct IridiumContext * context) {
  struct array * ary = array_new();
  struct array * ary2 = array_new();
  struct array * ary3 = array_new();

  ary = array_new();
  ary2 = array_new();
  array_set(ary, 0, 5);
  array_set(ary2, 1, 6);
  ary3 = array_merge(ary, ary2);
  
  assertEqual(array_get(ary3, 0), 5);
  assertEqual(array_get(ary3, 1), NULL);
  assertEqual(array_get(ary3, 2), 6);
  assertEqual(array_get(ary3, 3), NULL);

  return 0;
}

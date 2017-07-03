#include "../../src/lib/array.h"
#include "../test_helper.h"

int test() {
  struct array * ary = array_new();

  ary = array_new();
  array_set(ary, 0, 0x05);
  array_set(ary, 1, 0x07);

  assertEqual(array_pop(ary), 0x07);
  assertEqual(array_pop(ary), 0x05);
  assertEqual(array_pop(ary), NULL);

  return 0;
}


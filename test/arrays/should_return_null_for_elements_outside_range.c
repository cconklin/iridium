#include "../../src/lib/array.h"
#include "../test_helper.h"

int test() {
  struct array * ary = array_new();

  assertEqual(array_get(ary, 1000), NULL);  

  return 0;
}

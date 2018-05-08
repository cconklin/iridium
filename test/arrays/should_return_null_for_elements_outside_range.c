#include "../../iridium/include/shared/array.h"
#include "../test_helper.h"

int test(struct IridiumContext * context) {
  struct array * ary = array_new();

  assertEqual(array_get(ary, 1000), NULL);  

  return 0;
}

#include "../../src/array.h"
#include "../test_helper.h"

int main(int argc, char * argv[]) {
  struct array * ary = array_new();

  array_set(ary, 0, 1);
  assertEqual(array_get(array_copy(ary), 0), 1);

  return 0;
}
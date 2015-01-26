#include "../../src/lib/array.h"
#include "../test_helper.h"

int main(int argc, char * argv[]) {
  struct array * ary = array_new();

  assertEqual(array_get(ary, 0), NULL);

  return 0;
}
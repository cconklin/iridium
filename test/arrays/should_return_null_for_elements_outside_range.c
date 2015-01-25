#include "../../src/array.h"
#include "../test_helper.h"

int main(int argc, char * argv[]) {
  struct array * ary = array_new();

  assertEqual(array_get(ary, 1000), NULL);  

  return 0;
}
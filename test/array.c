#include "../src/array.h"

int main(int argc, char * argv[]) {
  struct array * ary = array_new();
  
  // it should return NULL for unset elements
  assert(array_get(ary, 0) == NULL);
  
  // it should return NULL for elements outside the original size
  assert(array_get(ary, 1000) == NULL);  
  
  // it should allow values to be set and retrieved at indecies
  array_set(ary, 1, 0x05);
  assert(array_get(ary, 1) == 0x05);
  
  // it should allow values to be set outside the original size
  array_set(ary, 100, 0x04);
  assert(array_get(ary, 100) == 0x04);
  
  return 0;
}
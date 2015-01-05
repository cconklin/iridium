#include "../src/array.h"
#include "test_helper.h"

int main(int argc, char * argv[]) {
  struct array * ary = array_new();
  struct array * ary2 = array_new();
  struct array * ary3 = array_new();
  
  // it should return NULL for unset elements
  assertEqual(array_get(ary, 0), NULL);
  
  // it should return NULL for elements outside the original size
  assertEqual(array_get(ary, 1000), NULL);  
  
  // it should allow values to be set and retrieved at indecies
  array_set(ary, 1, 0x05);
  assertEqual(array_get(ary, 1), 0x05);
  
  // it should allow values to be set outside the original size
  array_set(ary, 100, 0x04);
  assertEqual(array_get(ary, 100), 0x04);

  // it should allow values to be pushed to it
  ary = array_new();
  array_push(ary, 1);
  array_push(ary, 2);
  assertEqual(array_get(ary, 0), 1);
  assertEqual(array_get(ary, 1), 2);
  
  // it should allow values to be shifted from it
  assertEqual(array_shift(ary), 1);
  assertEqual(array_shift(ary), 2);
  assert(array_get(ary, 0) == NULL);

  // it should be copiable
  ary = array_new();
  array_set(ary, 0, 1);
  assertEqual(array_get(array_copy(ary), 0), 1);
  
  // it should be combinable
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
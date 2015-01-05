#include "../src/stack.h"
#include "test_helper.h"

int main(int argc, char * argv[]) {
  struct stack * st = stack_new();
  
  // It should allow pushing and peeking of values
  stack_push(st, 5);
  assertEqual(stack_top(st), 5);
  
  // It should allow popping of values
  assertEqual(stack_pop(st), 5);
  
  return 0;
}
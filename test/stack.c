#include "../src/stack.h"

int main(int argc, char * argv[]) {
  struct stack * st = stack_new();
  
  // It should allow pushing and peeking of values
  stack_push(st, 5);
  assert(stack_top(st) == 5);
  
  // It should allow popping of values
  assert(stack_pop(st) == 5);
  
  return 0;
}
#include "../../src/stack.h"
#include "../test_helper.h"

int main(int argc, char * argv[]) {
  struct stack * st = stack_new();

  assertEqual(stack_empty(st), 1);
  stack_push(st, 5);
  assertEqual(stack_empty(st), 0);

  return 0;
}
#include "../../src/lib/stack.h"
#include "../test_helper.h"

int test() {
  struct stack * st = stack_new();

  assertEqual(stack_empty(st), 1);
  stack_push(st, 5);
  assertEqual(stack_empty(st), 0);

  return 0;
}

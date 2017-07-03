#include "../../src/lib/stack.h"
#include "../test_helper.h"

int test() {
  struct stack * st = stack_new();

  stack_push(st, 5);
  assertEqual(stack_top(st), 5);

  return 0;
}

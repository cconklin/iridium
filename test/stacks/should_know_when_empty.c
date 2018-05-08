#include "../../iridium/include/shared/stack.h"
#include "../test_helper.h"

int test(struct IridiumContext * context) {
  struct stack * st = stack_new();

  assertEqual(stack_empty(st), 1);
  stack_push(st, 5);
  assertEqual(stack_empty(st), 0);

  return 0;
}

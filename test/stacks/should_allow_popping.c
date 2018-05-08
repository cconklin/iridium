#include "../../iridium/include/shared/stack.h"
#include "../test_helper.h"

int test(struct IridiumContext * context) {
  struct stack * st = stack_new();

  stack_push(st, 5);
  assertEqual(stack_pop(st), 5);
  assert(stack_empty(st));

  return 0;
}

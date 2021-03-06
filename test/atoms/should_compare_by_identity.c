#include "../../iridium/include/ir_object.h"
#include "../test_helper.h"

void setup(struct IridiumContext * context) {
  IR_early_init_Object(context);
  IR_init_Object(context);
}

int test(struct IridiumContext * context) {
  setup(context);
  object self = ATOM("self");
  object self_1 = ATOM("self");
  object a = ATOM("a");
  object b = ATOM("b");
  object a_1 = ATOM("a");
  object b_1 = ATOM("b");
  assertEqual(self, self_1);
  assertEqual(a, a_1);
  assertEqual(b, b_1);
  assertNotEqual(a, b);
  assertNotEqual(a, self);
  assertNotEqual(b, self);
  return 0;
}

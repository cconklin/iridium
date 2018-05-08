#include "../iridium/include/ir_object.h"
#include "test_helper.h"

void setup(struct IridiumContext * context) {
  IR_init_Object(context);
}

int test(struct IridiumContext * context) {
  setup(context);
  object a = ATOM("a");
  object attr = ATOM("attr");
  object val = ATOM("val");
  object internal_val = ATOM("internal_val");
  set_attribute(a, attr, PUBLIC, val);
  internal_set_attribute(a, attr, internal_val);

  assertEqual(get_attribute(a, attr, PUBLIC), val);
  assertEqual(internal_get_attribute(a, attr, object), internal_val);  
  return 0;
}

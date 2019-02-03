#include "../iridium/include/ir_object.h"
#include "test_helper.h"

void setup(struct IridiumContext * context) {
  IR_early_init_Object(context);
  IR_init_Object(context);
}

int test(struct IridiumContext * context) {
  setup(context);
  object a = ATOM("a");
  object attr = ATOM("attr");
  object val = ATOM("val");
  set_attribute(a, attr, PRIVATE, val);
  // Private attributes should return nil (later raise an exception) when a public attribute is sought
  assertEqual(get_attribute(a, attr, PUBLIC), NULL);
  assertEqual(get_attribute(a, attr, PRIVATE), val);  
  return 0;
}

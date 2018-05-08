#include "../../iridium/include/ir_object.h"
#include "../test_helper.h"
#include "setup.h"

int test(struct IridiumContext * context) {
  setup(context);

  object a = ATOM("a");
  object attr = ATOM("attr");
  object val = ATOM("val");

  // it should allow attributes to be set and retrieved
  set_attribute(a, attr, PUBLIC, val);
  assertEqual(get_attribute(a, attr, PUBLIC), val);

  return 0;
}

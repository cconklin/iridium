#include "../../iridium/include/ir_object.h"
#include "../test_helper.h"
#include "setup.h"

int test(struct IridiumContext * context) {
  setup(context);
  object a = ATOM("a");
  object instance_val = ATOM("instance_val");
  object instance_attr = ATOM("instance_attr");

  // it should allow attributes to be retrieved from a classes instance attributes
  set_instance_attribute(CLASS(Atom), instance_attr, PUBLIC, instance_val);
  assertEqual(get_attribute(a, instance_attr, PUBLIC), instance_val);

  return 0;
}

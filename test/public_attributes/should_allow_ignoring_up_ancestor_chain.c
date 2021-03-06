#include "../../iridium/include/ir_object.h"
#include "../test_helper.h"
#include "setup.h"

int test(struct IridiumContext * context) {
  setup(context);
  object a = ATOM("a");
  object instance_val = ATOM("instance_val");
  object instance_attr = ATOM("instance_attr");

  // it should allow objects to ignore attributes up the ancestor chain
  set_instance_attribute(CLASS(Atom), instance_attr, PUBLIC, instance_val);
  no_attribute(a, instance_attr);
  assertEqual(get_attribute(a, instance_attr, PUBLIC), NULL); // eventually rasie an exception

  return 0;
}

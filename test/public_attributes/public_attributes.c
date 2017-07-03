#include "../../src/object.h"
#include "../test_helper.h"
#include "setup.h"

int test() {
  setup();
  object b = ATOM("b");
  object instance_val = ATOM("instance_val");
  object instance_attr = ATOM("instance_attr");

  // it should allow objects to ignore instance attributes up the ancestor chain
  set_instance_attribute(CLASS(Atom), instance_attr, PUBLIC, instance_val);
  assertEqual(get_attribute(b, instance_attr, PUBLIC), instance_val);
  no_instance_attribute(CLASS(Atom), instance_attr);
  assertEqual(get_attribute(b, instance_attr, PUBLIC), NULL); // eventually rasie an exception

  return 0;
}

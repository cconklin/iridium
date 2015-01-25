#include "../../src/object.h"
#include "../test_helper.h"
#include "setup.h"

int main(int argc, char * argv[]) {
  setup();
  object a = ATOM("a");
  object instance_val = ATOM("instance_val");
  object instance_attr = ATOM("instance_attr");

  // it should allow attributes to be retrieved from a classes instance attributes
  set_instance_attribute(Atom, instance_attr, PUBLIC, instance_val);
  assertEqual(get_attribute(a, instance_attr, PUBLIC), instance_val);

  return 0;
}
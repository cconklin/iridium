#include "../src/object.h"
#include "test_helper.h"

void setup() {
  // Create Class
  Class = construct(Class);
  Class -> class = Class;

  // Create Atom
  Atom = construct(Class);

  // Create Object
  Object = construct(Class);
  set_attribute(Atom, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Class, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Object, ATOM("superclass"), PUBLIC, Object);
}

int main(int argc, char * argv[]) {
  setup();
  object a = ATOM("a");
  object b = ATOM("b");
  object attr = ATOM("attr");
  object val = ATOM("val");
  object instance_val = ATOM("instance_val");
  object instance_attr = ATOM("instance_attr");

  // it should allow attributes to be set and retrieved
  set_attribute(a, attr, PUBLIC, val);
  assertEqual(get_attribute(a, attr, PUBLIC), val);

  // it should allow attributes to be retrieved from a classes instance attributes
  set_instance_attribute(Atom, instance_attr, PUBLIC, instance_val);
  assertEqual(get_attribute(a, instance_attr, PUBLIC), instance_val);

  // it should allow objects to ignore attributes up the ancestor chain
  no_attribute(a, instance_attr);
  assertEqual(get_attribute(a, instance_attr, PUBLIC), NIL); // eventually rasie an exception

  // it should allow objects to ignore instance attributes up the ancestor chain
  assertEqual(get_attribute(b, instance_attr, PUBLIC), instance_val);
  no_instance_attribute(Atom, instance_attr);
  assertEqual(get_attribute(b, instance_attr, PUBLIC), NIL); // eventually rasie an exception

  return 0;
}
#include "../../src/object.h"
#include "../test_helper.h"

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
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
  object attr = ATOM("attr");
  object val = ATOM("val");
  set_attribute(a, attr, PUBLIC, val);
  assertEqual(get_attribute(a, attr, PUBLIC), val);
  return 0;
}
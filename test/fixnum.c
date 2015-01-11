#include "test_helper.h"
#include "../src/object.h"

void setup() {
  // Create Class
  Class = construct(Class);
  Class -> class = Class;

  // Create Atom
  Atom = construct(Class);
  
  // Create Fixnum
  Fixnum = construct(Class);

  // Create Object
  Object = construct(Class);
  set_attribute(Atom, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Class, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Fixnum, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Object, ATOM("superclass"), PUBLIC, Object);
}

int main(int argc, char * argv []) {
  object fixnum, fixnum_2;
  int integer;
  struct dict * bindings;

  setup();

  // it should round trip C integers
  fixnum = FIXNUM(5);
  assertEqual(INT(fixnum), 5);

  // it should be added to other fixnums
  fixnum = FIXNUM(5);
  fixnum_2 = FIXNUM(9);

  bindings = dict_new(ObjectHashsize);
  dict_set(bindings, ATOM("self"), fixnum);
  dict_set(bindings, ATOM("other"), fixnum_2);

  integer = INT(iridium_method_name(Fixnum, __plus__)(bindings));
  assertEqual(integer, 14);
  // Shouldn't mutate the arguments
  assertEqual(INT(fixnum), 5);
  assertEqual(INT(fixnum_2), 9);

  return 0;
}
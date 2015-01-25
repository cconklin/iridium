#include "../test_helper.h"
#include "../../src/object.h"
#include "setup.h"

int main(int argc, char * argv []) {
  object fixnum, fixnum_2;
  int integer;
  struct dict * bindings;

  setup();

  // it should be added to other fixnums
  fixnum = FIXNUM(5);
  fixnum_2 = FIXNUM(9);

  bindings = dict_new(ObjectHashsize);
  dict_set(bindings, ATOM("self"), fixnum);
  dict_set(bindings, ATOM("other"), fixnum_2);

  integer = INT(iridium_method_name(Fixnum, __plus__)(bindings));
  assertEqual(integer, 14);

  return 0;
}
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

  integer = INT(invoke(fixnum, "__plus__", array_push(array_new(), fixnum_2)));
  assertEqual(integer, 14);

  return 0;
}
#include "../test_helper.h"
#include "../../src/object.h"
#include "setup.h"

int test() {
  object fixnum, fixnum_2;
  int integer;

  setup();

  // it should be added to other fixnums
  fixnum = FIXNUM(5);
  fixnum_2 = FIXNUM(9);

  integer = INT(send(fixnum, "__add__", fixnum_2));
  assertEqual(integer, 14);

  return 0;
}

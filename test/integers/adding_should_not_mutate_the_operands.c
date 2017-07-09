#include "../test_helper.h"
#include "../../iridium/include/ir_object.h"
#include "setup.h"

int test() {
  object fixnum, fixnum_2;

  setup();

  fixnum = FIXNUM(5);
  fixnum_2 = FIXNUM(9);

  send(fixnum, "__add__", fixnum_2);

  // Shouldn't mutate the arguments
  assertEqual(INT(fixnum), 5);
  assertEqual(INT(fixnum_2), 9);

  return 0;
}

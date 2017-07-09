#include "../test_helper.h"
#include "../../iridium/include/ir_object.h"
#include "setup.h"

int test() {
  object fixnum;

  setup();

  // it should round trip C integers
  fixnum = FIXNUM(5);
  assertEqual(INT(fixnum), 5);

  return 0;
}

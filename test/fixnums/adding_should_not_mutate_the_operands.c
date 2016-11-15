#include "../test_helper.h"
#include "../../src/object.h"
#include "setup.h"

int main(int argc, char * argv []) {
  object fixnum, fixnum_2;

  setup();

  fixnum = FIXNUM(5);
  fixnum_2 = FIXNUM(9);

  send(fixnum, "__plus__", fixnum_2);

  // Shouldn't mutate the arguments
  assertEqual(INT(fixnum), 5);
  assertEqual(INT(fixnum_2), 9);

  return 0;
}
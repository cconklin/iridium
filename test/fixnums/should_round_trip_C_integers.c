#include "../test_helper.h"
#include "../../src/object.h"
#include "setup.h"

int main(int argc, char * argv []) {
  object fixnum;

  setup();

  // it should round trip C integers
  fixnum = FIXNUM(5);
  assertEqual(INT(fixnum), 5);

  return 0;
}
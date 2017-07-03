#include "../test_helper.h"
#include "../../src/object.h"
#include "setup.h"

int test() {

  object integer;
  char * c_integer;

  setup();

  integer = FIXNUM(-11);

  c_integer = C_STRING(send(integer, "to_s"));

  assertEqual(strcmp(c_integer, "-11"), 0);

  return 0;
}

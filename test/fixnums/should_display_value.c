#include "../test_helper.h"
#include "../../src/object.h"
#include "setup.h"

int main(int argc, char * argv []) {

  object integer;
  char * c_integer;

  setup();

  integer = FIXNUM(-11);

  c_integer = C_STRING(invoke(integer, "to_s", array_new()));

  assertEqual(strcmp(c_integer, "-11"), 0);

  return 0;
}

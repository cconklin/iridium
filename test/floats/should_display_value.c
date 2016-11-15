#include "../test_helper.h"
#include "../../src/float.h"
#include "setup.h"

int main(int argc, char * argv []) {

  object flt;
  char * c_flt;

  setup();

  flt = IR_FLOAT(5.0);

  c_flt = C_STRING(invoke(flt, "to_s", array_new()));

  assertEqual(strcmp(c_flt, "5.000000"), 0);

  return 0;
}

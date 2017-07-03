#include "../test_helper.h"
#include "../../src/float.h"
#include "setup.h"

int test() {
  object flt, flt_2;

  setup();

  flt = IR_FLOAT(5.0);
  flt_2 = IR_FLOAT(9.0);

  send(flt, "__plus__", flt_2);
  // Shouldn't mutate the arguments
  assertDoublesEqual(C_DOUBLE(flt), 5.0);
  assertDoublesEqual(C_DOUBLE(flt_2), 9.0);

  return 0;
}

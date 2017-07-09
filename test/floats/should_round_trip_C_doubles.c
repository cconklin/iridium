#include "../test_helper.h"
#include "../../iridium/include/ir_float.h"
#include "setup.h"

int test() {
  object flt;

  setup();

  // it should round trip C doubles
  flt = IR_FLOAT(5.0);
  assertDoublesEqual(C_DOUBLE(flt), 5.0);

  return 0;
}

#include "../test_helper.h"
#include "../../iridium/include/ir_float.h"
#include "setup.h"

int test(struct IridiumContext * context) {
  object flt;

  setup(context);

  // it should round trip C doubles
  flt = IR_FLOAT(5.0);
  assertDoublesEqual(C_DOUBLE(flt), 5.0);

  return 0;
}

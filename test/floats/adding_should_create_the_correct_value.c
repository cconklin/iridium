#include "../test_helper.h"
#include "../../iridium/include/ir_float.h"
#include "setup.h"

int test() {
  object flt, flt_2;
  object result;
  double val;

  setup();

  // it should be added to other flts
  flt = IR_FLOAT(5.0);
  flt_2 = IR_FLOAT(9.5);

  result = send(flt, "__plus__", flt_2);

  val = C_DOUBLE(result);
  assertDoublesEqual(val, 14.5);

  return 0;
}

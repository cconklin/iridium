#include "../test_helper.h"
#include "../../src/float.h"
#include "setup.h"

int main(int argc, char * argv []) {
  object flt, flt_2;
  object result;
  double val;
  struct dict * bindings;
  struct array * args = array_new();

  setup();

  // it should be added to other flts
  flt = IR_FLOAT(5.0);
  flt_2 = IR_FLOAT(9.5);

  array_push(args, flt_2);

  result = invoke(flt, "__plus__", args);

  val = C_DOUBLE(result);
  assertDoublesEqual(val, 14.5);

  return 0;
}

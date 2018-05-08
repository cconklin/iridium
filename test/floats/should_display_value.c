#include "../test_helper.h"
#include "../../iridium/include/ir_float.h"
#include "setup.h"

int test(struct IridiumContext * context) {

  object flt;
  char * c_flt;

  setup(context);

  flt = IR_FLOAT(5.0);

  c_flt = C_STRING(context, send(flt, "to_s"));

  assertEqual(strcmp(c_flt, "5.000000"), 0);

  return 0;
}

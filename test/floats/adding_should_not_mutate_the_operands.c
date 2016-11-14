#include "../test_helper.h"
#include "../../src/float.h"
#include "setup.h"

int main(int argc, char * argv []) {
  object flt, flt_2;
  struct dict * bindings;

  setup();

  flt = IR_FLOAT(5.0);
  flt_2 = IR_FLOAT(9.0);

  bindings = dict_new(ObjectHashsize);
  dict_set(bindings, ATOM("self"), flt);
  dict_set(bindings, ATOM("other"), flt_2);

  iridium_method_name(Float, __plus__)(bindings);
  // Shouldn't mutate the arguments
  assertDoublesEqual(C_DOUBLE(flt), 5.0);
  assertDoublesEqual(C_DOUBLE(flt_2), 9.0);

  return 0;
}
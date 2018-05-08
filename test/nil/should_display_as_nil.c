#include "../../iridium/include/ir_object.h"
#include "../test_helper.h"
#include "setup.h"

int test(struct IridiumContext * context) {
  char * c_nil;
  setup(context);
  c_nil = C_STRING(context, send(NIL, "to_s"));
  assertEqual(strcmp(c_nil, "nil"), 0);
}

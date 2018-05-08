#include "../../iridium/include/ir_object.h"
#include "../test_helper.h"

void setup(struct IridiumContext * context) {
  IR_init_Object(context);
}

int test(struct IridiumContext * context) {
  setup(context);
  object self = ATOM("self");
  object turtle = ATOM("turtle");
  char * c_self = C_STRING(context, send(self, "inspect"));
  char * c_turtle = C_STRING(context, send(turtle, "inspect"));

  assertEqual(strcmp(c_self, ":self"), 0);
  assertEqual(strcmp(c_turtle, ":turtle"), 0);
  return 0;
}

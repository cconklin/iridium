#include "../../src/object.h"
#include "../test_helper.h"

void setup() {
  IR_init_Object();
}

int main(int argc, char * argv[]) {
  setup();
  object self = ATOM("self");
  object turtle = ATOM("turtle");
  char * c_self = C_STRING(invoke(self, "to_s", array_new()));
  char * c_turtle = C_STRING(invoke(turtle, "to_s", array_new()));

  assertEqual(strcmp(c_self, ":self"), 0);
  assertEqual(strcmp(c_turtle, ":turtle"), 0);
  return 0;
}
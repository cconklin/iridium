#include "../../src/object.h"
#include "../test_helper.h"

void setup() {
  IR_init_Object();
}

int main(int argc, char * argv[]) {
  setup();
  object self = ATOM("self");
  object turtle = ATOM("turtle");
  char * c_self = C_STRING(send(self, "to_s"));
  char * c_turtle = C_STRING(send(turtle, "to_s"));

  assertEqual(strcmp(c_self, ":self"), 0);
  assertEqual(strcmp(c_turtle, ":turtle"), 0);
  return 0;
}
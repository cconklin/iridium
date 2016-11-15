#include "../../src/object.h"
#include "../test_helper.h"
#include "setup.h"

int main(int argc, const char * argv[]) {
  char * c_nil;
  setup();
  c_nil = C_STRING(send(NIL, "to_s"));
  assertEqual(strcmp(c_nil, "nil"), 0);
}

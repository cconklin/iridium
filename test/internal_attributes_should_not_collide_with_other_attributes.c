#include "../src/object.h"
#include "test_helper.h"

void setup() {
  IR_init_Object();
}

int main(int argc, char * argv[]) {
  setup();
  object a = ATOM("a");
  object attr = ATOM("attr");
  object val = ATOM("val");
  object internal_val = ATOM("internal_val");
  set_attribute(a, attr, PUBLIC, val);
  internal_set_attribute(a, attr, internal_val);

  assertEqual(get_attribute(a, attr, PUBLIC), val);
  assertEqual(internal_get_attribute(a, attr, object), internal_val);  
  return 0;
}
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
  set_attribute(a, attr, PRIVATE, val);
  // Private attributes should return nil (later raise an exception) when a public attribute is sought
  assertEqual(get_attribute(a, attr, PUBLIC), NULL);
  assertEqual(get_attribute(a, attr, PRIVATE), val);  
  return 0;
}

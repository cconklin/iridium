#include "../../src/object.h"
#include "../test_helper.h"
#include "setup.h"

iridium_method(Test, func) {
  return NIL;
}

int test() {
  object func, obj;
  setup();

  func = FUNCTION(ATOM("func"), NULL, dict_new(ObjectHashsize), iridium_method_name(Test, func));
  obj = ATOM("obj");
  set_attribute(obj, ATOM("f"), PUBLIC, func);

  assertEqual(invoke(obj, "f", array_new()), NIL);
  
  return 0;
}

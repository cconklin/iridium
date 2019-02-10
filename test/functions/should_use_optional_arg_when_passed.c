#include "../../iridium/include/ir_object.h"
#include "../test_helper.h"
#include "setup.h"

iridium_method(Test, func_required_and_optional) {
  assertEqual(s_local("a"), ATOM("a")); // required
  assertEqual(s_local("b"), ATOM("b")); // optional
  return NIL;
}

int test(struct IridiumContext * context) {
  object func, obj;
  struct IridiumArgument * a, * b;
  struct array * args;
  setup(context);

  // test with a required and optional arg passing both
  a = argument_new(ATOM("a"), NULL, 0);
  b = argument_new(ATOM("b"), ATOM("c"), 0);
  func = FUNCTION(ATOM("func"), list_cons(list_new(b), a), dict_new(ObjectHashsize), iridium_method_name(Test, func_required_and_optional));
  obj = ATOM("obj");
  set_attribute(obj, ATOM("f"), PUBLIC, func);
  args = array_new();
  array_push(args, ATOM("a"));
  array_push(args, ATOM("b"));
  // function f(a, b = :c)
  // obj.f(:a, :b) # > a = :a, b = :b
  invoke(context, obj, "f", args);
  
  return 0;
}

#include "../../iridium/include/ir_object.h"
#include "../test_helper.h"
#include "setup.h"

iridium_method(Test, func_splat_and_required) {
  // * args (splat)
  struct array * tuple_args = internal_get_attribute(local("args"), ATOM("array"), struct array *);
  assertEqual(array_get(tuple_args, 0), ATOM("a"));
  assertEqual(array_get(tuple_args, 1), ATOM("b"));
  assertEqual(local("b"), ATOM("c")); // optional
  return NIL;
}

int test(struct IridiumContext * context) {
  object func, obj;
  struct IridiumArgument * a, * b;
  struct array * args;
  setup(context);

  // test with a splatted args and an required arg, filling both
  a = argument_new(ATOM("args"), NULL, 1);
  b = argument_new(ATOM("b"), NULL, 0);
  func = FUNCTION(ATOM("func"), ARGLIST(a, b), dict_new(ObjectHashsize), iridium_method_name(Test, func_splat_and_required));
  obj = ATOM("obj");
  set_attribute(obj, ATOM("f"), PUBLIC, func);
  // function f(* args, b)
  // obj.f(:a, :b, :c) # > args = {:a, :b}, b = :c
  send(obj, "f", ATOM("a"), ATOM("b"), ATOM("c"));

  return 0;
}

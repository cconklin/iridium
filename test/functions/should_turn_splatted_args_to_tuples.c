#include "../../iridium/include/ir_object.h"
#include "../test_helper.h"
#include "setup.h"

iridium_method(Test, func_splat_and_optional) {
  // * args (splat)
  struct array * tuple_args = (struct array *) s_local("args")->immediate.ptr;
  assertEqual(array_get(tuple_args, 0), ATOM("a"));
  assertEqual(array_get(tuple_args, 1), ATOM("b"));
  assertEqual(s_local("b"), ATOM("c")); // optional
  return NIL;
}

int test(struct IridiumContext * context) {
  object func, obj;
  struct IridiumArgument * a, * b;
  struct array * args;
  setup(context);

  // test with a splatted args and an optional arg, filling both
  a = argument_new(ATOM("args"), NULL, 1);
  b = argument_new(ATOM("b"), ATOM("f"), 0);
  func = FUNCTION(ATOM("func"), list_cons(list_new(b), a), dict_new(ObjectHashsize), iridium_method_name(Test, func_splat_and_optional));
  obj = ATOM("obj");
  set_attribute(obj, ATOM("f"), PUBLIC, func);
  args = array_new();
  array_push(args, ATOM("a"));
  array_push(args, ATOM("b"));
  array_push(args, ATOM("c"));
  // function f(* args, b = :f)
  // obj.f(:a, :b, :c) # > args = {:a, :b}, b = :c
  invoke(context, obj, "f", args);

  return 0;
}

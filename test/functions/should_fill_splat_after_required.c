#include "../../src/object.h"
#include "../test_helper.h"
#include "setup.h"

iridium_method(Test, func_required_and_splat) {
  struct array * tuple_args = internal_get_attribute(local("args"), ATOM("array"), struct array *);
  assertEqual(local("a"), ATOM("a")); // required
  assertEqual(array_get(tuple_args, 0), ATOM("b")); // splat
  return NIL;
}

int test() {
  object func, obj;
  struct IridiumArgument * a, * b;
  struct array * args;
  setup();

  // test with a required and optional arg passing both
  a = argument_new(ATOM("a"), NULL, 0);
  b = argument_new(ATOM("args"), NULL, 1);
  func = FUNCTION(ATOM("func"), list_cons(list_new(b), a), dict_new(ObjectHashsize), iridium_method_name(Test, func_required_and_splat));
  obj = ATOM("obj");
  set_attribute(obj, ATOM("f"), PUBLIC, func);
  args = array_new();
  array_push(args, ATOM("a"));
  array_push(args, ATOM("b"));
  // function f(a, b = :c)
  // obj.f(:a, :b) # > a = :a, b = :b
  invoke(obj, "f", args);
  
  return 0;
}

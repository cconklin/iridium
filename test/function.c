#include "../src/object.h"
#include "test_helper.h"

void setup() {  
  object call, get;
  struct IridiumArgument * args, * name;
  
  // Allocate memory for the object
  Class = (object) GC_MALLOC(sizeof(struct IridiumObject));
  
  // Ensure that memory was allocated
  assert(Class);
  
  // Set the object's magic number
  Class -> magic = MAGIC;

  // Set the object's class to self (since this is a method on an instance of
  // Class, which implies that self is a class)
  Class -> class = Class;
  
  // Initialize the attribute dictionary
  Class -> attributes = dict_new(ObjectHashsize);
 
  // Create Atom
  Atom = construct(Class);
  
  // Create Function
  Function = construct(Class);
  
  // Create Object
  Object = construct(Class);
  set_attribute(Class, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Atom, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Function, ATOM("superclass"), PUBLIC, Object);
  
  args = argument_new(ATOM("args"), NULL, 1);
  call = FUNCTION(ATOM("__call__"), list_new(args), dict_new(ObjectHashsize), iridium_method_name(Function, __call__));
  
  name = argument_new(ATOM("name"), NULL, 0);
  get = FUNCTION(ATOM("__get__"), list_new(name), dict_new(ObjectHashsize), iridium_method_name(Object, __get__));
    
  set_instance_attribute(Function, ATOM("__call__"), PUBLIC, call);
  set_instance_attribute(Object, ATOM("__get__"), PUBLIC, get);
}

iridium_method(Test, func) {
  return NIL;
}

iridium_method(Test, func_required_and_optional) {
  assertEqual(local("a"), ATOM("a")); // required
  assertEqual(local("b"), ATOM("b")); // optional
  return NIL;
}

iridium_method(Test, func_splat_and_optional) {
  // * args (splat)
  struct array * tuple_args = internal_get_attribute(local("args"), ATOM("array"), struct array *);
  assertEqual(array_get(tuple_args, 0), ATOM("a"));
  assertEqual(array_get(tuple_args, 1), ATOM("b"));
  assertEqual(local("b"), ATOM("c")); // optional
  return NIL;
}


int main(int argc, char * argv[]) {
  object func, obj;
  struct IridiumArgument * a, * b;
  struct array * args;
  setup();
  func = FUNCTION(ATOM("func"), NULL, dict_new(ObjectHashsize), iridium_method_name(Test, func));
  assertEqual(func -> class, Function);
  assertEqual(get_attribute(func, ATOM("name"), PUBLIC), ATOM("func"));
  assertEqual(internal_get_attribute(func, ATOM("function"), object (*)(struct dict *))(dict_new(ObjectHashsize)), NIL); 
  
  // test the `invoke` macro
  obj = ATOM("obj");
  set_attribute(obj, ATOM("f"), PUBLIC, func);
  assertEqual(invoke(obj, "f", array_new()), NIL);
  
  // test with a required and optional arg passing only the required
  a = argument_new(ATOM("a"), NULL, 0);
  b = argument_new(ATOM("b"), ATOM("b"), 0);
  func = FUNCTION(ATOM("func"), list_cons(list_new(b), a), dict_new(ObjectHashsize), iridium_method_name(Test, func_required_and_optional));
  obj = ATOM("obj");
  set_attribute(obj, ATOM("f"), PUBLIC, func);
  args = array_new();
  array_push(args, ATOM("a"));
  // function f(a, b = :b)
  // obj.f(:a) # > a = :a, b = :b
  invoke(obj, "f", args);

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
  invoke(obj, "f", args);

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
  invoke(obj, "f", args);
  
  return 0;
}
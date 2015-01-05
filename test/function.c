#include "../src/object.h"
#include "test_helper.h"

void setup() {  
  object call;
  struct IridiumArgument * args;
  
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
  
  args = argument_new(ATOM("args"), NULL, 1);
  call = FUNCTION(ATOM("__call__"), list_new(args), dict_new(ObjectHashsize), iridium_method_name(Function, __call__));
  
  set_instance_attribute(Function, ATOM("__call__"), PUBLIC, call);
}

iridium_method(Test, func) {
  return NIL;
}

int main(int argc, char * argv[]) {
  object func, obj;
  setup();
  func = FUNCTION(ATOM("func"), NULL, dict_new(ObjectHashsize), iridium_method_name(Test, func));
  assertEqual(func -> class, Function);
  assertEqual(get_attribute(func, ATOM("name"), PUBLIC), ATOM("func"));
  assertEqual(internal_get_attribute(func, ATOM("function"), object (*)(struct dict *))(dict_new(ObjectHashsize)), NIL); 
  
  // test the `invoke` macro
  obj = ATOM("obj");
  set_attribute(obj, ATOM("f"), PUBLIC, func);
  assertEqual(invoke(obj, "f", array_new()), NIL);
  
  return 0;
}
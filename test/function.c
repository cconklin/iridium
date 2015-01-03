#include "../src/object.h"

void setup() {  
  
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
  
  Function = construct(Class);
}

iridium_method(Test, func) {
  return NIL;
}

int main(int argc, char * argv[]) {
  object func, obj;
  setup();
  func = FUNCTION(ATOM("func"), 0, 0, -1, dict_new(ObjectHashsize), iridium_method_name(Test, func));
  assert(func -> class == Function);
  assert(internal_get_attribute(func, ATOM("required"), int) == 0);
  assert(internal_get_attribute(func, ATOM("optional"), int) == 0);
  assert(internal_get_attribute(func, ATOM("splat"), char) == -1);
  assert(get_attribute(func, ATOM("name"), PUBLIC) == ATOM("func"));
  assert(internal_get_attribute(func, ATOM("function"), object (*)(struct dict *, struct array *))(dict_new(ObjectHashsize), array_new()) == NIL); 
  
  // test the `invoke` macro
  obj = ATOM("obj");
  set_attribute(obj, ATOM("f"), PUBLIC, func);
  assert(invoke(obj, "f", array_new()) == NIL);
  
  return 0;
}
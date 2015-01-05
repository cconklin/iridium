#include "../src/object.h"
#include "test_helper.h"

void define_Atom() {  
  
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
 
  // Allocate memory for the object
  Atom = (object) GC_MALLOC(sizeof(struct IridiumObject));
  
  // Ensure that memory was allocated
  assert(Atom);
  
  // Set the object's magic number
  Atom -> magic = MAGIC;

  // Set the object's class to self (since this is a method on an instance of
  // Class, which implies that self is a class)
  Atom -> class = Class;
  
  // Initialize the attribute dictionary
  Atom -> attributes = dict_new(ObjectHashsize);
  
  // Initialize the instance_attribute dictionary
  Atom -> instance_attributes = dict_new(ObjectHashsize);
  
  // Set Atom superclass to Atom (for testing purposes)
  set_attribute(Atom, ATOM("superclass"), PUBLIC, Atom);
}

int main(int argc, char * argv[]) {
  define_Atom();
  object a = ATOM("a");
  object attr = ATOM("attr");
  object val = ATOM("val");
  set_attribute(a, attr, PRIVATE, val);
  // Private attributes should return nil (later raise an exception) when a public attribute is sought
  assertEqual(get_attribute(a, attr, PUBLIC), NIL);
  assertEqual(get_attribute(a, attr, PRIVATE), val);  
  return 0;
}
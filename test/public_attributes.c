#include "../src/object.h"

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
  
}

int main(int argc, char * argv[]) {
  define_Atom();
  object a = ATOM("a");
  object attr = ATOM("attr");
  object val = ATOM("val");
  set_attribute(a, attr, PUBLIC, val);
  assert(get_attribute(a, attr, PUBLIC) == val);
  return 0;
}
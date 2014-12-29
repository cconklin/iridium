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
  object self = ATOM("self");
  object self_1 = ATOM("self");
  object a = ATOM("a");
  object b = ATOM("b");
  object a_1 = ATOM("a");
  object b_1 = ATOM("b");
  assert(self == self_1);
  assert(a == a_1);
  assert(b == b_1);
  assert(a != b);
  assert(a != self);
  assert(b != self);
  return 0;
}
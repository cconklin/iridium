object MyException, AnotherException;

void setup() {
  // Create Class
  Class = construct(Class);
  Class -> class = Class;

  // Create Atom
  Atom = construct(Class);
  
  // Create MyException
  MyException = construct(Class);
  AnotherException = construct(Class);
  
  // Create Object
  Object = construct(Class);
  // TODO make inherit Exception
  set_attribute(MyException, ATOM("superclass"), PUBLIC, Object);
  set_attribute(AnotherException, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Atom, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Class, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Object, ATOM("superclass"), PUBLIC, Object);
  
  // Initialize the exception stack
  _exception_frames = stack_new();
}
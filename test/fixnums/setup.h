void setup() {
  // Create Class
  Class = construct(Class);
  Class -> class = Class;

  // Create Atom
  Atom = construct(Class);
  
  // Create Fixnum
  Fixnum = construct(Class);

  // Create Object
  Object = construct(Class);
  set_attribute(Atom, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Class, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Fixnum, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Object, ATOM("superclass"), PUBLIC, Object);
}
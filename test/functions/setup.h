void setup() {  
  object call, get;
  struct IridiumArgument * args, * name;
  
  // Create Class
  Class = construct(Class);
  Class -> class = Class;

  // Create Atom
  Atom = construct(Class);

  // Create Object
  Object = construct(Class);
  set_attribute(Class, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Atom, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Object, ATOM("superclass"), PUBLIC, Object);

  // Create Function
  Function = construct(Class);
  set_attribute(Function, ATOM("superclass"), PUBLIC, Object);

  args = argument_new(ATOM("args"), NULL, 1);
  call = FUNCTION(ATOM("__call__"), list_new(args), dict_new(ObjectHashsize), iridium_method_name(Function, __call__));

  name = argument_new(ATOM("name"), NULL, 0);
  get = FUNCTION(ATOM("__get__"), list_new(name), dict_new(ObjectHashsize), iridium_method_name(Object, __get__));

  set_instance_attribute(Function, ATOM("__call__"), PUBLIC, call);
  set_instance_attribute(Object, ATOM("__get__"), PUBLIC, get);
}
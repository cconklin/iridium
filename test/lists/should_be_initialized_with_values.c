#include "../test_helper.h"
#include "../../src/list.h"

int main(int argc, char * argv[]) {
  // Create Class
  Class = construct(Class);
  Class -> class = Class;

  // Create Atom
  Atom = construct(Class);

  // Create Object
  Object = construct(Class);
  set_attribute(Atom, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Class, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Object, ATOM("superclass"), PUBLIC, Object);

  List = construct(Class);

  struct array * args = array_new();
  array_set(args, 0, ATOM("a"));
  array_set(args, 1, ATOM("b"));
  object l = construct(List);
  struct dict * bindings = dict_new(ObjectHashsize);
  dict_set(bindings, ATOM("self"), l);
  dict_set(bindings, ATOM("args"), TUPLE(args));

  object result = iridium_method_name(List, initialize)(bindings);

  // It should have its head be the first element passed
  assertEqual(list_head(internal_get_attribute(result, ATOM("list"), struct list *)), ATOM("a"));
  // It should have its second element be the second element passed
  assertEqual(list_head(list_tail(internal_get_attribute(result, ATOM("list"), struct list *))), ATOM("b"));

  return 0;
}


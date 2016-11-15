#include "object.h"

#ifndef IRIDIUM_LIST
#define IRIDIUM_LIST

// Class name
object List;

// List#initialize
// Creates a list
// List.new(1, 2, 3) # => [1, 2, 3]
// Args:
//    args (tuple)
iridium_method(List, initialize) {
  object self = local("self"); // Receiver
  object args = local("args"); // Tuple of values
  struct array * arg_values = destructure(array_new(), args);
  struct list * l;
  if (arg_values -> length == 0) {
    l = list_new(NULL); // Empty list
  } else {
    l = list_new(array_pop(arg_values));
    while (arg_values -> length) {
      l = list_cons(l, array_pop(arg_values));
    }
  }
  internal_set_attribute(self, ATOM("list"), l);
  return self;
}

// List Init
void IR_init_List() {
  struct IridiumArgument * args = argument_new(ATOM("args"), NULL, 1);
  object list_initialize;
  List = invoke(Class, "new", array_push(array_new(), IR_STRING("List")));
  list_initialize = FUNCTION(ATOM("initialize"), list_new(args), dict_new(ObjectHashsize), iridium_method_name(List, initialize));
  set_instance_attribute(List, ATOM("initialize"), PUBLIC, list_initialize);
}

#endif


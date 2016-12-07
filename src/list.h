#include "object.h"

#ifndef IRIDIUM_LIST
#define IRIDIUM_LIST

// Class name
object CLASS(List);

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
    l = NULL; // Empty list
  } else {
    l = list_new(array_pop(arg_values));
    while (arg_values -> length) {
      l = list_cons(l, array_pop(arg_values));
    }
  }
  internal_set_attribute(self, ATOM("list"), l);
  return NIL;
}

// List#reduce
// Left fold for lists
iridium_method(List, reduce) {
  object self = local("self");
  object accumulator = local("accumulator");
  object fn = local("fn");
  object element;
  struct list * lst = internal_get_attribute(self, ATOM("list"), struct list *);
  struct array * args;
  while (lst) {
    element = list_head(lst);
    args = array_new();
    array_set(args, 0, element);
    array_set(args, 1, accumulator);
    accumulator = calls(fn, args);
    lst = list_tail(lst);
  }
  return accumulator;
}

// List Init
void IR_init_List() {
  struct IridiumArgument * args = argument_new(ATOM("args"), NULL, 1);
  object list_initialize;
  CLASS(List) = send(CLASS(Class), "new", IR_STRING("List"));
  list_initialize = FUNCTION(ATOM("initialize"), ARGLIST(args), dict_new(ObjectHashsize), iridium_method_name(List, initialize));
  set_instance_attribute(CLASS(List), ATOM("initialize"), PUBLIC, list_initialize);
  DEF_METHOD(CLASS(List), "reduce", ARGLIST(argument_new(ATOM("accumulator"), NULL, 0), argument_new(ATOM("fn"), NULL, 0)), iridium_method_name(List, reduce));
}

#endif


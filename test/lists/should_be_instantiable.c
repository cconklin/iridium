#include "../test_helper.h"
#include "../../src/list.h"

int main(int argc, char * argv[]) {
  IR_init_Object();

  List = construct(Class);

  struct array * args = array_new();
  object l = construct(List);
  struct dict * bindings = dict_new(ObjectHashsize);
  dict_set(bindings, ATOM("self"), l);
  dict_set(bindings, ATOM("args"), TUPLE(args));

  object result = iridium_method_name(List, initialize)(bindings);

  // It should have an (empty) internal list
  assertEqual(list_head(internal_get_attribute(result, ATOM("list"), struct list *)), NULL);

  return 0;
}


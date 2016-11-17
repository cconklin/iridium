#include "../test_helper.h"
#include "../../src/list.h"

int main(int argc, char * argv[]) {
  IR_init_Object();
  IR_init_List();

  object result = send(CLASS(List), "new", ATOM("a"), ATOM("b"));

  // It should have its head be the first element passed
  assertEqual(list_head(internal_get_attribute(result, ATOM("list"), struct list *)), ATOM("a"));
  // It should have its second element be the second element passed
  assertEqual(list_head(list_tail(internal_get_attribute(result, ATOM("list"), struct list *))), ATOM("b"));

  return 0;
}


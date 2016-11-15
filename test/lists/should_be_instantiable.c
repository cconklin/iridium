#include "../test_helper.h"
#include "../../src/list.h"

int main(int argc, char * argv[]) {
  IR_init_Object();
  IR_init_List();

  object result = invoke(List, "new", array_new());

  // It should have an (empty) internal list
  assertEqual(list_head(internal_get_attribute(result, ATOM("list"), struct list *)), NULL);

  return 0;
}


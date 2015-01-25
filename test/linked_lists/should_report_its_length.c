#include "../../src/linked_list.h"
#include "../test_helper.h"

int main (int argc, char const *argv[]) {
  struct list * l = list_new(5);

  assertEqual(list_length(l), 1);

  return 0;
}
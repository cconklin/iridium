#include "../../iridium/include/ir_object.h"
#include "../test_helper.h"
#include "setup.h"

iridium_method(Test, func) {
  return NIL;
}

int test(struct IridiumContext * context) {
  object func, anon, meth;
  setup(context);

  object Owner, Child;
  
  Owner = send(CLASS(Class), "new", IR_STRING("Owner"));
  Child = send(CLASS(Class), "new", IR_STRING("Child"), Owner);

  // Name does not have owner when anonymous
  anon = FUNCTION(ATOM("anon"), NULL, dict_new(ObjectHashsize), iridium_method_name(Test, func));
  char * anon_name = C_STRING(context, send(anon, "to_s"));
  char expected_anon_name[100];
  sprintf(expected_anon_name, "#<Function anon:%p>", anon);
  assertEqual(strcmp(anon_name, expected_anon_name), 0);

  // Name contains owner when owned
  func = FUNCTION(ATOM("func"), NULL, dict_new(ObjectHashsize), iridium_method_name(Test, func));
  set_attribute(Owner, ATOM("func"), PUBLIC, func);
  char * func_name = C_STRING(context, send(pubget(Child, "func"), "to_s"));
  char expected_func_name[100];
  sprintf(expected_func_name, "#<Function Owner.func:%p>", func);
  assertEqual(strcmp(func_name, expected_func_name), 0);

  // Name contains class/module when instance
  meth = FUNCTION(ATOM("func"), NULL, dict_new(ObjectHashsize), iridium_method_name(Test, func));
  set_instance_attribute(Owner, ATOM("func"), PUBLIC, func);
  object instance = send(Child, "new");
  sprintf(expected_func_name, "#<Function Owner#func:%p>", func);
  func_name = C_STRING(context, send(pubget(instance, "func"), "to_s"));
  assertEqual(strcmp(func_name, expected_func_name), 0);

  return 0;
}

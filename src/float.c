#include "float.h"

/* Float.new
 *
 * Creates a float
 *
 * Float.new(2.5) # => 2.5
 *
 * Uses double precision floating point numbers under the hood
 *
 * Arguments:
 * - val (iridium float)
 */
iridium_classmethod(Float, new) {
  object val = local("val"); // Float value
  return val; // Since Floats are immutable, we can return the original
}

/* IR_FLOAT
 *
 * Converts a double into an iridium float
 *
 * Arguments:
 * - val (C double)
 *
 * Returns:
 * - Iridium Float
 */
object IR_FLOAT(double val) {
  object flt = construct(CLASS(Float));
  internal_set_flt(flt, ATOM("value"), val);
  return flt;
}

/* C_DOUBLE
 *
 * Converts an iridium float into a native double
 *
 * Arguments:
 * - flt (iridium float)
 *
 * Returns:
 * - C double
 */
double C_DOUBLE(object flt) {
  return internal_get_flt(flt, ATOM("value"), double);
}

/* __plus__
 *
 * Add two iridium floats
 *
 * Arguments:
 * - self (iridium float)
 * - other (iridium float)
 *
 * Returns:
 * - Iridium Float
 */
iridium_method(Float, __plus__) {
  double l, r;
  object self = local("self"); // Receiver
  object other = local("other"); // Other float
  l = C_DOUBLE(self);
  r = C_DOUBLE(other);
  return IR_FLOAT(l + r);
}

/* to_s
 *
 * Display the float as a literal value
 *
 * Arguments:
 * - self
 *
 * Returns:
 * - Iridium String
 */
iridium_method(Float, to_s) {
  object self = local("self");
  double val = C_DOUBLE(self);
  char buffer[100];
  char * str;
  sprintf(buffer, "%lf", val);
  str = GC_MALLOC((strlen(buffer) + 1) * sizeof(char));
  assert(str);
  strcpy(str, buffer);
  return IR_STRING(str);
}

// Hash method for Floats
iridium_method(Float, hash) {
  object self = local("self");
  double dval = C_DOUBLE(self);
  long long int val = *((long long int *) &dval);
  return FIXNUM((val << 19) + (val >> 3));
}

/* Setup Code */
void IR_init_Float() {
  CLASS(Float) = send(CLASS(Class), "new", IR_STRING("Float"));

  object new_func = FUNCTION(ATOM("new"), ARGLIST(argument_new(ATOM("val"), NULL, 0)), dict_new(ObjectHashsize), iridium_classmethod_name(Float, new));
  set_attribute(CLASS(Float), ATOM("new"), PUBLIC, new_func);

  object plus_func = FUNCTION(ATOM("__plus__"), ARGLIST(argument_new(ATOM("other"), NULL, 0)), dict_new(ObjectHashsize), iridium_method_name(Float, __plus__));
  set_instance_attribute(CLASS(Float), ATOM("__plus__"), PUBLIC, plus_func);

  object to_s_func = FUNCTION(ATOM("to_s"), ARGLIST(), dict_new(ObjectHashsize), iridium_method_name(Float, to_s));
  set_instance_attribute(CLASS(Float), ATOM("to_s"), PUBLIC, to_s_func);

  DEF_METHOD(CLASS(Float), "hash", ARGLIST(), iridium_method_name(Float, hash));

  define_constant(ATOM("Float"), CLASS(Float));
}

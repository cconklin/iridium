#include "object.h"

#ifndef IRIDIUM_FLOAT
#define IRIDIUM_FLOAT

// Class name
object Float;

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
  object flt = construct(Float);
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

/* Setup Code */
void IR_init_Float() {
  Float = invoke(Class, "new", array_push(array_new(), IR_STRING("Float")));
  // Float = construct(Class);
  // set_attribute(Float, ATOM("superclass"), PUBLIC, Object);
  object new_func = FUNCTION(ATOM("new"), list_new(argument_new(ATOM("val"), NULL, 0)), dict_new(ObjectHashsize), iridium_classmethod_name(Float, new));
  set_attribute(Float, ATOM("new"), PUBLIC, new_func);
  object plus_func = FUNCTION(ATOM("__plus__"), list_new(argument_new(ATOM("other"), NULL, 0)), dict_new(ObjectHashsize), iridium_method_name(Float, __plus__));
  set_instance_attribute(Float, ATOM("__plus__"), PUBLIC, plus_func);
}

#endif


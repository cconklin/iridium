#include "object.h"

#ifndef IRIDIUM_FLOAT
#define IRIDIUM_FLOAT

// Class name
object Float;

/* double2ptr
 *
 * Converts a double into a pointer value
 *
 * Arguments:
 * - val (double)
 *
 * Returns:
 * - (void *) with the same binary value as +val+
 */
void * double2ptr(double val) {
  return (void *) *((unsigned long long *) &val);
}

/* ptr2double
 *
 * Converts a pointer into a double value
 *
 * Arguments:
 * - ptr (void *)
 *
 * Returns:
 * - (double) with the same binary value as +ptr+
 */
double ptr2double(void * ptr) {
  return (double) *((double *) & ptr);
}

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
  internal_set_attribute(flt, ATOM("value"), double2ptr(val));
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
  return ptr2double(internal_get_attribute(flt, ATOM("value"), void *));
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
  l = ptr2double(internal_get_attribute(self, ATOM("value"), void *));
  r = ptr2double(internal_get_attribute(other, ATOM("value"), void *));
  return IR_FLOAT(l + r);
}

/* Setup Code */
void IR_init_Float() {
  Float = invoke(Class, "new", array_new());
  // Float = construct(Class);
  // set_attribute(Float, ATOM("superclass"), PUBLIC, Object);
  object new_func = FUNCTION(ATOM("new"), list_new(argument_new(ATOM("val"), NULL, 0)), dict_new(ObjectHashsize), iridium_classmethod_name(Float, new));
  set_attribute(Float, ATOM("new"), PUBLIC, new_func);
  object plus_func = FUNCTION(ATOM("__plus__"), list_new(argument_new(ATOM("other"), NULL, 0)), dict_new(ObjectHashsize), iridium_method_name(Float, __plus__));
  set_instance_attribute(Float, ATOM("__plus__"), PUBLIC, plus_func);
}

#endif


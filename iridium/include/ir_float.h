#include "ir_object.h"

#ifndef IRIDIUM_FLOAT
#define IRIDIUM_FLOAT

// Class name
object CLASS(Float);

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
object IR_FLOAT(double val);

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
double C_DOUBLE(object flt);

/* Setup Code */
void IR_init_Float(struct IridiumContext * context);

#endif

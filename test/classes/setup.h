#include "../../iridium/include/ir_object.h"

void setup(struct IridiumContext * context)
{
    IR_early_init_Object(context);
    IR_init_Object(context);
}


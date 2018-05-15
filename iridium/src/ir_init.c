#include "ir_init.h"
#include "atoms.h"

void IR_INIT(struct IridiumContext * context) {
    IR_early_init_context(context);
    IR_init_Object(context);
    // set the _raised to NIL now that NIL is defined
    context->_raised = NIL;
    IR_init_Float(context);
    IR_init_String(context);
    IR_init_Dictionary(context);
    IR_init_File(context);
    IR_init_Queue(context);
    IR_init_Thread(context);
    // IR_CORE_INIT is a main function of the compiled
    IR_CORE_INIT(context);
}

iridium_classmethod(ir_main, to_s) {
    object self = local("self");
    char buffer[100];
    sprintf(buffer, "#<Object(main):%p>", self);
    char * c_str = GC_MALLOC((strlen(buffer) + 1) * sizeof(char));
    assert(c_str);
    strcpy(c_str, buffer);
    return IR_STRING(c_str); 
}

object IR_MAIN_OBJECT(struct IridiumContext * context) {
    object ir_main = send(CLASS(Object), "new");
    object main_to_s = FUNCTION(L_ATOM(inspect), ARGLIST(), dict_new(ObjectHashsize), iridium_classmethod_name(ir_main, to_s));
    set_attribute(ir_main, L_ATOM(inspect), PUBLIC, main_to_s);
    return ir_main;
}


#include "ir_init.h"

void IR_INIT(void) {
    IR_init_Object();
    IR_init_Float();
    IR_init_String();
    IR_init_Dictionary();
    IR_init_File();
    IR_init_Queue();
    handler_id = 0; // Set the first handler id
    _exception_frames = stack_new(); // Initialize the exception stack
    // IR_CORE_INIT is a main function of the compiled
    IR_CORE_INIT();
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

object IR_MAIN_OBJECT(void) {
    object ir_main = send(CLASS(Object), "new");
    object main_to_s = FUNCTION(ATOM("inspect"), ARGLIST(), dict_new(ObjectHashsize), iridium_classmethod_name(ir_main, to_s));
    set_attribute(ir_main, ATOM("inspect"), PUBLIC, main_to_s);
    return ir_main;
}


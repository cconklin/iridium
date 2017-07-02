#include "iridium.h"

void IR_INIT(void) {
    IR_init_Object();
    IR_init_Float();
    IR_init_String();
    IR_init_Dictionary();
    IR_init_File();
    _exception_frames = stack_new(); // Initialize the exception stack
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

int main(int argc, char ** argv) {
    // TODO ARGV constant
    IR_INIT();
    // TODO: only catch children of Exception
    struct list * main_exceptions = list_new(EXCEPTION(CLASS(Object), 1));
    exception_frame e = ExceptionHandler(main_exceptions, 0, 0, 0);
    switch (setjmp(e -> env)) {
        case 0:
            ir_user_main();
            return 0;
        case 1:
            // Any uncaught exception
            printf("%s: %s\n", C_STRING(send(_raised->class, "to_s")), C_STRING(send(send(_raised, "reason"), "to_s")));
            return 1;
    }
}


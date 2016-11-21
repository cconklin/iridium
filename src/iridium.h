#include "object.h"
#include "float.h"
#include "list.h"
#include "string.h"

void ir_user_main();

void IR_INIT(void) {
    IR_init_Object();
    IR_init_List();
    IR_init_Float();
    IR_init_String();
}

iridium_classmethod(ir_main, to_s) {
    object self = local("self");
    char buffer[100];
    sprintf(buffer, "#<Object(main):%p>", self);
    char * c_str = GC_MALLOC((strlen(buffer) + 1) * sizeof(char));
    assert(c_str);
    return IR_STRING(c_str); 
}

object IR_MAIN_OBJECT(void) {
    object ir_main = send(CLASS(Object), "new");
    object main_to_s = FUNCTION(ATOM("to_s"), ARGLIST(), dict_new(ObjectHashsize), iridium_classmethod_name(ir_main, to_s));
    set_attribute(ir_main, ATOM("to_s"), PUBLIC, main_to_s);
    return ir_main;
}

int main(int argc, char ** argv) {
    // TODO ARGV constant
    IR_INIT();
    ir_user_main();
    return 0;
}


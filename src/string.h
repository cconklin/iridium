#include "object.h"

iridium_method(String, __add__)
{
    object self = local("self");
    object other = local("other");
    char * self_str = C_STRING(self);
    char * other_str = C_STRING(other);
    assert(other_str); // If +other+ is a stringy type, it won't be NULL
    char * result_str = GC_MALLOC((strlen(self_str) + strlen(other_str) + 1)*sizeof(char));
    assert(result_str);
    strcpy(result_str, self_str);
    strcat(result_str, other_str);
    return IR_STRING(result_str);
}

void IR_init_String(void)
{
    DEF_METHOD(CLASS(String), "__add__", ARGLIST(argument_new(ATOM("other"), NULL, 0)), iridium_method_name(String, __add__));
}

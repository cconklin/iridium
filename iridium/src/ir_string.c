#include "ir_string.h"
#include "atoms.h"

iridium_method(String, __add__)
{
    object self = local(self);
    object other = local(other);
    char * self_str = C_STRING(context, self);
    char * other_str = C_STRING(context, other);
    if (!other_str) // If +other+ is a stringy type, it won't be NULL
    {
        char error_buf[1000];
        sprintf(error_buf, "%s is not a %s", C_STRING(context, send(other, "inspect")), C_STRING(context, send(self->class, "inspect")));
        char * error = GC_MALLOC((strlen(error_buf)+1) * sizeof(char));
        strcpy(error, error_buf);
        RAISE(send(CLASS(TypeError), "new", IR_STRING(error)));
    }
    char * result_str = GC_MALLOC((strlen(self_str) + strlen(other_str) + 1)*sizeof(char));
    assert(result_str);
    strcpy(result_str, self_str);
    strcat(result_str, other_str);
    return IR_STRING(result_str);
}

iridium_method(String, __eq__)
{
    object self = local(self);
    object other = local(other);
    if (self -> class != other -> class) {
      // Both had better be strings
      return ir_cmp_false;
    }
    char * self_str = C_STRING(context, self);
    char * other_str = C_STRING(context, other);
    assert(other_str); // If +other+ is a stringy type, it won't be NULL
    return (strcmp(self_str, other_str) == 0) ? ir_cmp_true : ir_cmp_false;
}

// Hash method for strings
iridium_method(String, hash) {
  object self = local(self);
  unsigned long hash = 5381;
  int c;
  char * str = C_STRING(context, self);
  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c; // hash * 33 + c
  }
  return FIXNUM(hash);
}

void IR_init_String(struct IridiumContext * context)
{
    DEF_METHOD(CLASS(String), "__add__", ARGLIST(argument_new(L_ATOM(other), NULL, 0)), iridium_method_name(String, __add__));
    DEF_METHOD(CLASS(String), "__eq__", ARGLIST(argument_new(L_ATOM(other), NULL, 0)), iridium_method_name(String, __eq__));
    DEF_METHOD(CLASS(String), "hash", ARGLIST(), iridium_method_name(String, hash));
}


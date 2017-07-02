#include "object.h"
#include "float.h"
#include "string.h"
#include "dictionary.h"
#include "file.h"
#pragma once

struct array * ir_context_stack;

object IR_MAIN_OBJECT(void);
object ir_user_main(void);
object IR_CORE_INIT();
int main(int argc, char ** argv);

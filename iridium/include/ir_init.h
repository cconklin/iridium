#include "ir_object.h"
#include "ir_float.h"
#include "ir_string.h"
#include "ir_dictionary.h"
#include "ir_file.h"
#include "ir_queue.h"
#include "ir_thread.h"
#pragma once

struct array * ir_context_stack;

object IR_MAIN_OBJECT(void);
object ir_user_main(void);
object IR_CORE_INIT(void);
void IR_INIT(void);


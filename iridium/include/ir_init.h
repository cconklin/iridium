#include "ir_object.h"
#include "ir_float.h"
#include "ir_string.h"
#include "ir_dictionary.h"
#include "ir_file.h"
#include "ir_queue.h"
#include "ir_thread.h"
#include "ir_regex.h"
#pragma once

struct array * ir_context_stack;

object IR_MAIN_OBJECT(struct IridiumContext *);
object ir_user_main(struct IridiumContext *);
object IR_CORE_INIT(struct IridiumContext *);
void IR_INIT(struct IridiumContext *);
void IR_EARLY_INIT(struct IridiumContext *);


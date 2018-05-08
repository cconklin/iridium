#include "../../test_helper.h"
#include "../../../iridium/include/ir_object.h"
#include "../setup.h"

int returns_in_begin(struct IridiumContext * context) {
  struct list * exceptions = list_new(EXCEPTION(MyException, 1));
  exception_frame e = ExceptionHandler(context, exceptions, 1, 0, 0);
  switch (setjmp(e -> env)) {
    case 0:
      // begin
      // ...
      return_in_begin_block(context, (object) -1);
      END_BEGIN(context, e);
    case ENSURE_JUMP:
      // ensure
      // ...
      return_in_begin_block(context, (object) 1);
      END_ENSURE(context, e);
    case FRAME_RETURN:
      if (e -> count == 0) {
        return e -> return_value;
      } else {
        return_in_begin_block(context, e -> return_value);
      }
      break;
  }
}

int returns_in_nested_begin(struct IridiumContext * context) {
  struct list * exceptions = list_new(EXCEPTION(MyException, 1));
  exception_frame e = ExceptionHandler(context, exceptions, 1, 0, 0);
  exception_frame e_2;
  switch (setjmp(e -> env)) {
    case 0:
      // begin
      // ...
      exceptions = list_new(EXCEPTION(AnotherException, 1));
      e_2 = ExceptionHandler(context, exceptions, 1, 0, 1);
      switch (setjmp(e_2 -> env)) {
        case 0:
          return_in_begin_block(context, (object) -1);
          END_BEGIN(context, e_2);
        case ENSURE_JUMP:
          return_in_begin_block(context, (object) 1);
          END_ENSURE(context, e_2);
        case FRAME_RETURN:
          if (e_2 -> count == 0) {
            return e_2 -> return_value;
          } else {
            return_in_begin_block(context, e_2 -> return_value);
          }
          break;
      }
      END_BEGIN(context, e);
    case ENSURE_JUMP:
      // ensure
      return_in_begin_block(context, (object) 2);
      END_ENSURE(context, e);
    case FRAME_RETURN:
      if (e -> count == 0) {
        return e -> return_value;
      } else {
        return_in_begin_block(context, e -> return_value);
      }
      break;
  }
}

int returns_in_nested_begin_without_returning_ensure(struct IridiumContext * context) {
  struct list * exceptions = list_new(EXCEPTION(MyException, 1));
  exception_frame e = ExceptionHandler(context, exceptions, 1, 0, 0);
  exception_frame e_2;
  switch (setjmp(e -> env)) {
    case 0:
      // begin
      // ...
      exceptions = list_new(EXCEPTION(AnotherException, 1));
      e_2 = ExceptionHandler(context, exceptions, 1, 0, 1);
      switch (setjmp(e_2 -> env)) {
        case 0:
          return_in_begin_block(context, (object) -1);
          END_BEGIN(context, e_2);
        case ENSURE_JUMP:
          END_ENSURE(context, e_2);
        case FRAME_RETURN:
          if (e_2 -> count == 0) {
            return e_2 -> return_value;
          } else {
            return_in_begin_block(context, e_2 -> return_value);
          }
          break;
      }
      END_BEGIN(context, e);
    case ENSURE_JUMP:
      // ensure
      END_ENSURE(context, e);
    case FRAME_RETURN:
      if (e -> count == 0) {
        return e -> return_value;
      } else {
        return_in_begin_block(context, e -> return_value);
      }
      break;
  }
}

int test(struct IridiumContext * context) {
  setup(context);
  
  // it binds the raised exception
  // Raising an exception
  // Create the exception frame with no ensure
  
  // it should have returns in ensures take priority over returns in the begin
  assertEqual(returns_in_begin(context), 1);
  assert(stack_empty(context->_exception_frames));
  
  // it should give the shallowest ensure highest priority in a return
  assertEqual(returns_in_nested_begin(context), 2);
  assert(stack_empty(context->_exception_frames));

  // it should return what a begin returned if ensure does not return
  assertEqual(returns_in_nested_begin_without_returning_ensure(context), -1);
  assert(stack_empty(context->_exception_frames));

  return 0;
}

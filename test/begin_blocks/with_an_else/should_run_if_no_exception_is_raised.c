#include "../../test_helper.h"
#include "../../../iridium/include/ir_object.h"
#include "../setup.h"

int test(struct IridiumContext * context) {
  setup(context);
  
  // x is a flag to indicate whether a jump occurred
  int x = 0;
  struct list * exceptions = list_new(EXCEPTION(MyException, 1));
  exception_frame e = ExceptionHandler(context, exceptions, 0, 1, 0);

  switch (setjmp(e -> env)) {
    case 0:
      // begin
      // ...
      END_BEGIN(context, e);
    case ELSE_JUMP:
      // else
      // ...
      x = 2;
      END_ELSE(context, e);
  }
  assert(stack_empty(context->_exception_frames));
  assertEqual(x, 2);
  
  return 0;
}

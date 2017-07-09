#include "../../test_helper.h"
#include "../../../iridium/include/ir_object.h"
#include "../setup.h"

int test() {
  setup();
  
  // x is a flag to indicate whether a jump occurred
  int x = 0;
  struct list * exceptions = list_new(EXCEPTION(MyException, 1));
  object exc = construct(MyException);
  exception_frame e = ExceptionHandler(exceptions, 0, 1, 0);

  switch (setjmp(e -> env)) {
    case 0:
      // begin
      // ...
      handleException(exc);
      END_BEGIN(e);
    case 1:
      // rescue MyException
      // ...
      END_RESCUE(e);      
    case ELSE_JUMP:
      // else
      // ...
      assertNotReaches();
      END_ELSE(e);
  }
  assert(stack_empty(_exception_frames));
  
  return 0;
}

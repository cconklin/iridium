#include "../test_helper.h"
#include "../../src/object.h"
#include "setup.h"

int main(int argc, char * argv[]) {
  setup();
  
  // x is a flag to indicate whether a jump occurred
  int x = 0;
  struct list * exceptions = list_new(EXCEPTION(MyException, 1));
  object exc = construct(MyException);
  exception_frame e = ExceptionHandler(exceptions, 0, 0);
  
  switch (setjmp(e -> env)) {
    case 0:
      // begin
      // ...
      handleException(exc);
      // Once the exception is handled, it should not return
      assertNotReaches();
      END_BEGIN(e);
    case 1:
      // rescue MyException
      // ...
      // Change the value of x to indicate that we reached here
      x = 1;
      assertEqual(exc, _raised);
      END_RESCUE(e);
  }
  assert(stack_empty(_exception_frames));
  assertEqual(x, 1);
  
  return 0;
}
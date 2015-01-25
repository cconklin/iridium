#include "../../test_helper.h"
#include "../../../src/object.h"
#include "../setup.h"

int main(int argc, char * argv[]) {
  setup();
  
  // x is a flag to indicate whether a jump occurred
  int x = 0;
  struct list * exceptions = list_new(EXCEPTION(MyException, 1));
  exception_frame e = ExceptionHandler(exceptions, 0, 0);
  int y = 0;
  exception_frame e_2;
  
  switch (setjmp(e -> env)) {
    case 0:
      // begin
      // ...
      exceptions = list_new(EXCEPTION(AnotherException, 1));
      e_2 = ExceptionHandler(exceptions, 1, 1);
      switch (setjmp(e_2 -> env)) {
        case 0:      
          handleException(construct(MyException));
          // Once the exception is handled, it should not return
          assertNotReaches();
          END_BEGIN(e_2);
        case ENSURE_JUMP:
          y = 1;
          END_ENSURE;
      }
      END_BEGIN(e);
    case 1:
      // rescue MyException
      // ...
      // Change the value of x to indicate that we reached here
      x = 1;
      END_RESCUE(e);
  }
  assert(stack_empty(_exception_frames));
  assertEqual(x, 1);
  assertEqual(y, 1);
  
  return 0;
}
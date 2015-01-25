#include "../test_helper.h"
#include "../../src/object.h"
#include "setup.h"

int main(int argc, char * argv[]) {
  setup();
  
  // x is a flag to indicate whether a jump occurred
  int x = 0;
  struct list * exceptions = list_new(EXCEPTION(MyException, 1));
  
  // With no exception raised
  // Create the exception frame with no ensure
  exception_frame e = ExceptionHandler(exceptions, 0, 0);
  switch (setjmp(e -> env)) {
    case 0:
      // begin
      // ...
      x = 1;
      END_BEGIN(e);
    case 1:
      // rescue MyException
      // ...
      assertNotReaches();
      END_RESCUE(e);
  }
  assert(stack_empty(_exception_frames));
  assertEqual(x, 1);
  
  return 0;
}
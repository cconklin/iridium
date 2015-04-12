#include "../../test_helper.h"
#include "../../../src/object.h"
#include "../setup.h"

int main(int argc, char * argv[]) {
  setup();
  
  // x is a flag to indicate whether a jump occurred
  int x = 0;
  struct list * exceptions = list_new(EXCEPTION(MyException, 1));
  exception_frame e = ExceptionHandler(exceptions, 0, 1, 0);

  switch (setjmp(e -> env)) {
    case 0:
      // begin
      // ...
      END_BEGIN(e);
    case ELSE_JUMP:
      // else
      // ...
      x = 2;
      END_ELSE(e);
  }
  assert(stack_empty(_exception_frames));
  assertEqual(x, 2);
  
  return 0;
}
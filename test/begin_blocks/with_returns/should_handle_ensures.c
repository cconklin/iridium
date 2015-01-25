#include "../../test_helper.h"
#include "../../../src/object.h"
#include "../setup.h"

int returns_in_begin() {
  struct list * exceptions = list_new(EXCEPTION(MyException, 1));
  exception_frame e = ExceptionHandler(exceptions, 1, 0);
  switch (setjmp(e -> env)) {
    case 0:
      // begin
      // ...
      return_in_begin_block();
      return 0;
      END_BEGIN(e);
    case ENSURE_JUMP:
      // ensure
      // ...
      return_in_begin_block();
      return 1;
      END_ENSURE;
  }
}

int returns_in_nested_begin() {
  struct list * exceptions = list_new(EXCEPTION(MyException, 1));
  exception_frame e = ExceptionHandler(exceptions, 1, 0);
  exception_frame e_2;
  switch (setjmp(e -> env)) {
    case 0:
      // begin
      // ...
      exceptions = list_new(EXCEPTION(AnotherException, 1));
      e_2 = ExceptionHandler(exceptions, 1, 1);
      switch (setjmp(e_2 -> env)) {
        case 0:      
          return_in_begin_block();
          return 0;
          END_BEGIN(e_2);
        case ENSURE_JUMP:
          return_in_begin_block();
          return 1;
          END_ENSURE;
      }
      END_BEGIN(e);
    case ENSURE_JUMP:
      // ensure
      return_in_begin_block();
      return 2;
      END_ENSURE;
  }
}

int main(int argc, char * argv[]) {
  setup();
  
  // it binds the raised exception
  // Raising an exception
  // Create the exception frame with no ensure
  
  // it should have returns in ensures take priority over returns in the begin
  assertEqual(returns_in_begin(), 1);
  
  // it should give the shallowest ensure highest priority in a return
  assertEqual(returns_in_nested_begin(), 2);
  
  return 0;
}
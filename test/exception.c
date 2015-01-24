#include "test_helper.h"
#include "../src/object.h"

object MyException, AnotherException;

void setup() {
  // Create Class
  Class = construct(Class);
  Class -> class = Class;

  // Create Atom
  Atom = construct(Class);
  
  // Create MyException
  MyException = construct(Class);
  AnotherException = construct(Class);
  
  // Create Object
  Object = construct(Class);
  // TODO make inherit Exception
  set_attribute(MyException, ATOM("superclass"), PUBLIC, Object);
  set_attribute(AnotherException, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Atom, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Class, ATOM("superclass"), PUBLIC, Object);
  set_attribute(Object, ATOM("superclass"), PUBLIC, Object);
  
  // Initialize the exception stack
  _exception_frames = stack_new();
}

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
  
  // Raising an exception
  // Create the exception frame with no ensure
  x = 0;
  exceptions = list_new(EXCEPTION(MyException, 1));
  e = ExceptionHandler(exceptions, 0, 0);
  switch (setjmp(e -> env)) {
    case 0:
      // begin
      // ...
      handleException(construct(MyException));
      // Once the exception is handled, it should not return
      assertNotReaches();
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
  
  // With an ensure
  x = 0;
  exceptions = list_new(EXCEPTION(MyException, 1));
  e = ExceptionHandler(exceptions, 1, 0);
  switch (setjmp(e -> env)) {
    case 0:
      // begin
      // ...
      END_BEGIN(e);
    case ENSURE_JUMP:
      // ensure
      // ...
      x = 2;
      END_ENSURE;
  }
  assert(stack_empty(_exception_frames));
  assertEqual(x, 2);
  
  // Two levels with an ensure on the inner level
  int y = 0;
  exception_frame e_2;
  x = 0;
  
  exceptions = list_new(EXCEPTION(MyException, 1));
  e = ExceptionHandler(exceptions, 0, 0);
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
  
  // it binds the raised exception
  // Raising an exception
  // Create the exception frame with no ensure
  x = 0;
  object exc = construct(MyException);
  exceptions = list_new(EXCEPTION(MyException, 1));
  e = ExceptionHandler(exceptions, 0, 0);
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
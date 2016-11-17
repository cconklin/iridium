object MyException, AnotherException;

void setup() {
  // Init
  IR_init_Object();

  // Create MyException
  MyException = invoke(CLASS(Class), "new", array_push(array_new(), IR_STRING("MyException")));
  AnotherException = invoke(CLASS(Class), "new", array_push(array_new(), IR_STRING("AnotherException")));
  
  // Initialize the exception stack
  _exception_frames = stack_new();
}

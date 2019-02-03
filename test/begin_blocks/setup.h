object MyException, AnotherException;

void setup(struct IridiumContext * context) {
  // Init
  IR_early_init_Object(context);
  IR_init_Object(context);

  // Create MyException
  MyException = invoke(context, CLASS(Class), "new", array_push(array_new(), IR_STRING("MyException")));
  AnotherException = invoke(context, CLASS(Class), "new", array_push(array_new(), IR_STRING("AnotherException")));
  
  // Initialize the exception stack
  context->_exception_frames = stack_new();
}

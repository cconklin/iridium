void setup(struct IridiumContext * context) {  
  IR_early_init_Object(context);
  __IR_USER_ATOM_INIT();
  IR_init_Object(context);
  IR_init_Float(context);
  IR_init_String(context);
  IR_init_Dictionary(context);
  IR_init_File(context);
  IR_init_Queue(context);
  IR_init_Regex(context);
  IR_init_Thread(context);
}

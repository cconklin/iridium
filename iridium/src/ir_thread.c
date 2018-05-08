#include "ir_thread.h"

void * ir_dispatch_thread(void * _arg) {
  struct ThreadArg * arg = (struct ThreadArg *) _arg;
  object thr = arg->thread;
  struct IridiumContext * context = arg->context;
  free(_arg);
  // Invoke the thread function
  object rval = send(thr, "fn");
  set_attribute(thr, ATOM("return_value"), PUBLIC, rval);
  pthread_exit(NULL);
}

object CLASS(Thread);

iridium_method(Thread, initialize) {
  object self = local("self");
  object fn = local("fn");
  pthread_t * thr = GC_MALLOC(sizeof(pthread_t));
  pthread_attr_t * attr = GC_MALLOC(sizeof(pthread_attr_t));
  assert(thr);
  assert(attr);
  internal_set_attribute(self, ATOM("thr"), thr);
  internal_set_attribute(self, ATOM("thr_attr"), attr);
  pthread_attr_init(attr);
  pthread_attr_setdetachstate(attr, PTHREAD_CREATE_JOINABLE);

  set_attribute(self, ATOM("fn"), PUBLIC, fn);
  set_attribute(self, ATOM("return_value"), PUBLIC, NIL);

  struct ThreadArg * arg = (struct ThreadArg *) malloc(sizeof(struct ThreadArg));
  assert(arg);
  arg->thread = self;
  arg->context = context;

  // Dispatch the thread
  int rc = pthread_create(thr, attr, ir_dispatch_thread, (void *) arg);

  // Raise an exception if the thread failed to be dispatched
  if (rc) {
    RAISE(send(CLASS(Exception), "new", IR_STRING("Could not create thread")));
  }

  return NIL;
}

iridium_method(Thread, join) {
  object self = local("self");
  pthread_t * thr = internal_get_attribute(self, ATOM("thr"), pthread_t *);
  int rc = pthread_join(*thr, NULL);
  if (rc) {
    RAISE(send(CLASS(Exception), "new", IR_STRING("Could not join thread")));
  }
  return get_attribute(self, ATOM("return_value"), PUBLIC);
}

void IR_init_Thread(struct IridiumContext * context) {
  CLASS(Thread) = send(CLASS(Class), "new", IR_STRING("Thread"));

  DEF_METHOD(CLASS(Thread), "initialize", ARGLIST(argument_new(ATOM("fn"), NULL, 0)), iridium_method_name(Thread, initialize));
  DEF_METHOD(CLASS(Thread), "join", ARGLIST(), iridium_method_name(Thread, join));

  define_constant(ATOM("Thread"), CLASS(Thread));
}


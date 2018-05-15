#include "ir_thread.h"
#include "atoms.h"

void * ir_dispatch_thread(void * _thr) {
  object thr = (object) _thr;
  struct IridiumContext ctx;
  IR_init_context(&ctx);
  struct IridiumContext * context = &ctx;
  struct ThreadStatus * status = (struct ThreadStatus *) GC_MALLOC(sizeof(struct ThreadStatus));
  assert(status);
  // Invoke the thread function
  struct list * thr_exceptions = list_new(EXCEPTION(CLASS(Object), 1));
  exception_frame e = ExceptionHandler(context, thr_exceptions, 0, 0, 0);
  switch (setjmp(e->env)) {
  case 0:
    status->is_ok = 1;
    status->result = send(thr, "fn");
    break;
  case 1:
    // Any uncaught exception
    status->is_ok = 0;
    status->result = context->_raised;
    break;
  }
  internal_set_attribute(thr, L_ATOM(result), status);
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
  internal_set_attribute(self, L_ATOM(thr), thr);
  internal_set_attribute(self, L_ATOM(thr_attr), attr);
  pthread_attr_init(attr);
  pthread_attr_setdetachstate(attr, PTHREAD_CREATE_JOINABLE);

  set_attribute(self, L_ATOM(fn), PUBLIC, fn);
  internal_set_attribute(self, L_ATOM(result), NULL);

  // Dispatch the thread
  int rc = pthread_create(thr, attr, ir_dispatch_thread, (void *) self);

  // Raise an exception if the thread failed to be dispatched
  if (rc) {
    RAISE(send(CLASS(Exception), "new", IR_STRING("Could not create thread")));
  }

  return NIL;
}

iridium_method(Thread, join) {
  object self = local("self");
  pthread_t * thr = internal_get_attribute(self, L_ATOM(thr), pthread_t *);
  int rc = pthread_join(*thr, NULL);
  if (rc) {
    RAISE(send(CLASS(Exception), "new", IR_STRING("Could not join thread")));
  }
  struct ThreadStatus * status = internal_get_attribute(self, L_ATOM(result), struct ThreadStatus *);
  if (status->is_ok) {
    return status->result;
  } else {
    // Move the thread's context onto our context
    struct stack * stacktrace = internal_get_attribute(status->result, L_ATOM(stacktrace), struct stack *);
    struct stack * rstacktrace = stack_new();
    while (!stack_empty(stacktrace)) {
      stack_push(rstacktrace, stack_pop(stacktrace));
    }
    while (!stack_empty(rstacktrace)) {
      stack_push(context->stacktrace, stack_pop(rstacktrace));
    }
    // Raise the exception from the thread
    RAISE(status->result);
  }
}

void IR_init_Thread(struct IridiumContext * context) {
  CLASS(Thread) = send(CLASS(Class), "new", IR_STRING("Thread"));

  DEF_METHOD(CLASS(Thread), "initialize", ARGLIST(argument_new(L_ATOM(fn), NULL, 0)), iridium_method_name(Thread, initialize));
  DEF_METHOD(CLASS(Thread), "join", ARGLIST(), iridium_method_name(Thread, join));

  define_constant(L_ATOM(Thread), CLASS(Thread));
}


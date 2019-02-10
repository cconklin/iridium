#include "ir_queue.h"
#include "atoms.h"

iridium_method(Queue, initialize) {
  object self = local(self);
  struct list * lst = NULL;
  internal_set_attribute(self, L_ATOM(list), lst);
  return NIL;
}

/* Enqueue an element into the queue, returning that element
 *
 * x = Queue.new()
 * => #<Queue>
 * x.enqueue("foo")
 * => "foo"
 */
iridium_method(Queue, enqueue) {
  object self = local(self);
  object obj = local(obj);
  struct list * lst = internal_get_attribute(self, L_ATOM(list), struct list *);
  if (lst) {
    list_extend(lst, (void *) obj);
  } else {
    lst = list_cons(lst, (void *) obj);
    internal_set_attribute(self, L_ATOM(list), lst);
  }
  return obj;
}

/* Dequeue an element from the queue, raising an Queue::Empty exception if empty
 */
iridium_method(Queue, dequeue) {
  object self = local(self);
  struct list * lst = internal_get_attribute(self, L_ATOM(list), struct list *);
  object result = NIL;
  object reason = NULL;
  if (lst) {
    result = list_head(lst);
    lst = list_tail(lst);
    internal_set_attribute(self, L_ATOM(list), lst);
  } else {
    reason = send(self, "inspect");
    reason = send(reason, "__add__", IR_STRING(" is empty"));
    RAISE(send(CLASS(EmptyQueueError), "new", reason));
  }
  return result;
}

/* Returns the number of elements in the queue
 */
iridium_method(Queue, length) {
  object self = local(self);
  struct list * lst = internal_get_attribute(self, L_ATOM(list), struct list *);
  unsigned int len = list_length(lst);
  return FIXNUM(len);
}

void IR_init_Queue(struct IridiumContext * context) {
  CLASS(Queue) = send(CLASS(Class), "new", IR_STRING("Queue"));
  CLASS(EmptyQueueError) = send(CLASS(Class), "new", IR_STRING("Queue.Empty"), CLASS(Exception));

  DEF_METHOD(CLASS(Queue), "initialize", ARGLIST(), iridium_method_name(Queue, initialize));
  DEF_METHOD(CLASS(Queue), "enqueue", ARGLIST(argument_new(L_ATOM(obj), NULL, 0)), iridium_method_name(Queue, enqueue));
  DEF_METHOD(CLASS(Queue), "dequeue", ARGLIST(), iridium_method_name(Queue, dequeue));

  set_attribute(CLASS(Queue), L_ATOM(Empty), PUBLIC, CLASS(EmptyQueueError));
  define_constant(L_ATOM(Queue), CLASS(Queue));
}


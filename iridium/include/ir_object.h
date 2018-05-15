#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <setjmp.h>
#include <stdarg.h>
#include <pthread.h>

#ifndef OBJECT_H
#define OBJECT_H

extern pthread_rwlock_t LOCK;

#include "garbage_collector.h"

// Use the array library to get args
#include "shared/array.h"
// Use the list library for module lookup
#include "shared/linked_list.h"
// Use the dict library for attributes
#include "shared/dict.h"
// Use the str_dict library for atoms
#include "shared/str_dict.h"
// Use the stack library for exceptions
#include "shared/stack.h"

// Magic number to identify objects
#define MAGIC 0x0EFFACED

// Enum for attribute access levels
#define PUBLIC    0
#define PRIVATE   1

// Macros for methods and method names
#define iridium_method_name(class, name) Iridium_##class##_##name
#define iridium_classmethod_name(class, name) IridiumClassmethod_##class##_##name

#define iridium_method(class, name) object iridium_method_name(class, name)(struct IridiumContext * context, struct dict * locals)
#define iridium_classmethod(class, name) object iridium_classmethod_name(class, name)(struct IridiumContext * context, struct dict * locals)


// Struct underlying all Iridium objects
typedef struct IridiumObject {
  // Calling card of all objects
  unsigned long long int magic;
  // Pointer to the class description (is also an Iridium Object)
  struct IridiumObject * class;
  // Dictionary of attributes
  struct dict * attributes;
  // Dictionary of internal attributes (hidden from an Iridium program)
  struct dict * internal_attributes;
  // Dictionary of attributes for instances
  struct dict * instance_attributes;
  // List of included modules
  struct list * included_modules;
  // List of extended modules
  struct list * extended_modules;
} * object;

// Struct underlying all Iridium object attributes
typedef struct IridiumAttribute {
  unsigned char access;
  unsigned char real; // set to 0 to stop attribute lookup
  void * value;
} * iridium_attribute;

// Struct underlying all function argument lists
struct IridiumArgument {
  object name; // Atom
  object default_value; // Object, NULL if required
  char splat; // Whether the argument takes multiple values
};

struct IridiumContext {
    // Thread-global variable to hold exceptions
    // Will ALWAYS have a top level handler which will terminate the Iridium program
    struct stack * _exception_frames;
    // Global variable to hold the exception being raised
    object _raised;
    // Global variable to indicate if an exception is being handled
    int _rescuing;
    // Location that an ensure jumps to if it completes normally when an exception is raised
    jmp_buf _handler_env;
    
    // Unique identifier of each handler
    int handler_id;

    // Stack Trace
    int trace;
    struct stack * stacktrace;
};

void IR_early_init_context(struct IridiumContext * context);
void IR_init_context(struct IridiumContext * context);
struct list * _ARGLIST(int unused, ...);

char * function_name(struct IridiumContext * context, object self);

#define ARGLIST(...) _ARGLIST(0, ##__VA_ARGS__, 0)

// Hashsize for object attribute hashes
#define ObjectHashsize 30

// Macro to identify objects
#define isObject(obj) (obj && ((struct IridiumObject *) obj) -> magic == MAGIC)

// Macro to identify types
#define isA(obj, klass) (isObject(obj) && ((struct IridiumObject *) obj) -> class == klass)

// Macro for changing self
// Returns a local dictionary containing `self` with the passed `value`
#define bind_self(value) dict_with(dict_new(ObjectHashsize), ATOM("self"), value)

// TODO define NIL macro
#define NIL (nil ? nil : (nil = create_nil()))

#define CLASS(name) ir_cmp_##name

// Macro for superclasses
#define superclass(class) get_attribute(class, ATOM("superclass"), PUBLIC)

// Macro for defining methods
#define DEF_METHOD(receiver, name, arglist, func) \
  set_instance_attribute(receiver, ATOM(name), PUBLIC, \
      FUNCTION(ATOM(name), arglist, dict_new(ObjectHashsize), func))
#define DEF_FUNCTION(receiver, name, arglist, func) \
  set_attribute(receiver, ATOM(name), PUBLIC, \
      FUNCTION(ATOM(name), arglist, dict_new(ObjectHashsize), func))

// Current Class/Module
object ir_context;

// Declarations

object CLASS(Module);
object CLASS(Class);
object CLASS(Object);
object CLASS(Function);
object CLASS(Atom);
object CLASS(Array);
object CLASS(NilClass);
object CLASS(Integer);
object CLASS(String);
object CLASS(Boolean);

object CLASS(Exception);
object CLASS(AttributeError);
object CLASS(NameError);
object CLASS(TypeError);
object CLASS(ArgumentError);

object ir_cmp_true;
object ir_cmp_false;

extern object nil;

object _exprval;
#define TRUTHY(expr) ((_exprval = expr) == nil ? 0 : (_exprval != ir_cmp_false))
#define FALSY(expr) (!TRUTHY(expr))

iridium_classmethod(Class, new);
iridium_method(Class, new);
iridium_method(Class, inspect);
iridium_method(Object, inspect);
iridium_method(Object, to_s);
iridium_method(Object, initialize);
iridium_method(Object, __get__);
iridium_method(Object, __set__);
iridium_method(Function, __call__);
iridium_classmethod(Atom, new);

object get_attribute(object, void *, unsigned char);
object ATOM(char * name);
object create_self_atom();
object create_nil();
object construct(object class);
object FUNCTION(object name, struct list * args, struct dict * bindings, object (* func)(struct IridiumContext * context, struct dict *));
object ARRAY(struct array * values);
int INT(object);
object FIXNUM(int);
object IR_STRING(char *);
char * C_STRING(struct IridiumContext * context, object);
void handleException(struct IridiumContext *, object);

#define RAISE(exc) handleException(context, exc)

char * str(object);
struct array * destructure(struct IridiumContext *, struct array *, object);
struct dict * process_args(struct IridiumContext *, object, struct array *);

extern struct dict * constants;

struct IridiumArgument * argument_new(object name, object default_value, char splat);

#define send(obj, name, ...) _send(context, obj, name , ##__VA_ARGS__, NULL)

// Function to determine object inheritance
int kindOf(object obj, object class);

// instance_attribute lookup
// Looks for an instance attribute of a Class, looking at included modules and superclasses, NULL on failure
// Helper function of attribute_lookup

// Macro for when no attribute is found
#define no_result ((! result) || (result -> access > access))

// Macro for whether a module list has any modules
#define modules_present(mod_list) ((mod_list) ? (list_length(mod_list) > 0) : 0 )

iridium_attribute instance_attribute_lookup(object receiver, void * attribute, unsigned char access);

// get_attribute
// Returns the value of an object attribute, nil on failure

// Declaration of helper function
iridium_attribute attribute_lookup(object, void *, unsigned char);

object get_attribute(object receiver, void * attribute, unsigned char access);
// Helper function
iridium_attribute attribute_lookup(object receiver, void * attribute, unsigned char access);

// internal_get_attribute
// Returns an attribute of obj which is INTERNAL
#define internal_get_attribute(obj, attr, cast) ((cast)(dict_get(((object) obj) -> internal_attributes, attr)))
#define internal_get_flt(obj, attr, cast) ((dict_get_flt(((object) obj) -> internal_attributes, attr)))
#define internal_get_integral(obj, attr, cast) ((cast)(dict_get_integral(((object) obj) -> internal_attributes, attr)))

// internal_set_attribute
// mutates the receiver, setting attribute with INTERNAL access to a new value
// NOTE: could run into issue where attribute is set before with a lower access (which means that an attribute that ought to be internal isn't)
#define internal_set_attribute(receiver, attribute, value) dict_set(receiver -> internal_attributes, attribute, value)
#define internal_set_flt(receiver, attribute, value) dict_set_flt(receiver -> internal_attributes, attribute, value)
#define internal_set_integral(receiver, attribute, value) dict_set_integral(receiver -> internal_attributes, attribute, value)

// set_attribute
// mutates the receiver, setting the attribute with `access` to a new value
object set_attribute(object receiver, void * attribute, unsigned char access, object value);

// remove an attribute from an object (make it so that the object no longer sees an attribute)
object no_attribute(object receiver, void * attribute);

// remove an instance attribute from an object (make it so that the object's instances no longer sees an attribute)
object no_instance_attribute(object receiver, void * attribute);
// set_instance_attribute
// mutates the receiver, setting the instance attribute with access to a new value
object set_instance_attribute(object receiver, void * attribute, unsigned char access, object value);

// Locals
#define local(name) _local(context, locals, ATOM(name))
object _local(struct IridiumContext * context, struct dict * locals, object atm);

#define set_local(name, value) _set_local(locals, ATOM(name), value)
void _set_local(struct dict * locals, object name, object value);

// function_bind
// Bind locals to functions
// Does not mutate the receiver
// accepts: func (object, Iridium Function), locals (struct dict *)
// returns: object (Iridirum Function)
object function_bind(object func, struct dict * locals);

// Unified construction sequence of objects (used in ALL constructors)

object construct(object class);

// Function for Function calls
// Translates to:
//    obj.name(args)
// Iridium_Object_get() will retun a bound function to the `obj` #"name" method (obj will automatically be passed)
// Invoke function with handle `name` by getting the function from the object `obj`, and bind it to `self` before passing it to the C Function for Iridium Function calls.
object invoke(struct IridiumContext * context, object obj, char * name, struct array * args);

object _send(struct IridiumContext * context, object obj, char * name, ...);

#define pubget(obj, name) get_attribute(obj, ATOM(name), PUBLIC)

// Function for generic invocations
// Used by the translator

object calls(struct IridiumContext * context, object callable, struct array * args);

// Converts iridium strings to C strings
char * str(object string);

// FUNCTION helper
// args:
//  name (Atom)
//  args (struct list * (of arguments))
//  bindings (struct dict *)
//  func (iridium_method)
//  Convert a C function to an Iridium Function
object FUNCTION(object name, struct list * args, struct dict * bindings, object (* func)(struct IridiumContext *, struct dict *));


extern void * ATOM_TABLE_KEY;
#define ATOM_HASHSIZE 100

struct dict * ATOM_TABLE();

void add_atom(char * name, object atom);

// Create the `:string` atom
object create_str_atom();

// Create the `:self` atom
object create_self_atom();

// Exception Handling

struct Exception {
  // Exception Class
  object class;
  // Value that setjmp will return when it is jumped to
  int jump_location;
};

typedef struct ExceptionFrame {
  // Unique identifier for this handler
  int id;
  // Index into stacktrace
  int stacktrace_idx;
  // Used to indicate the index of this exception frame
  // When a iridium function is invoked, this counter starts at zero
  // Each successive exception frame in this iridium function has a higher
  // index than the previous. That way, if a return happens inside a begin..end
  // it is possible to determine how many ensures to handle by popping elements
  // until `count` is 0. Returns outside of a begin..end block will not interact
  // with the exceptions stack
  int count;
  // Jump value for ensure
  // Make nonzero if there is an ensure, zero otherwise
  int ensure;
  // Jump value for else
  // Make nonzero if there is an else, zero otherwise
  int else_block;
  // List of exceptions to handle
  struct list * exceptions;
  // Location to jump to
  jmp_buf env;
  // Has it already rescued? Used to determine if the handler should be ignored
  // because of an exception within a rescue/else
  int already_rescued;
  // Has it already rescued? Used to determine if the handler should be ignored
  // because of an exception within a ensure
  int in_ensure;
  // Return value from begin block/rescue
  object return_value;
} * exception_frame;

/*
  Basic Exception Handler structure

  // Rescue MyException
  struct list * exceptions = list_new(EXCEPTION(MyException, 1));
  // Create the exception frame with no ensure
  // _handler_count is defined to be 0 at the top of every function
  exception_frame e = ExceptionHandler(exceptions, 0, _handler_count ++);
  switch (setjmp(e -> env)) {
    case 0:
      // begin
      // ...
      END_BEGIN(e);
    case 1:
      // rescue MyException
      // ...
      END_RESCUE(e);
    case ENSURE_JUMP:
      // ...
      END_ENSURE(e);
    case FRAME_RETURN:
      return e -> return_value;
  }
  _handler_count --;
*/

#define ENSURE_JUMP -1
#define END_ENSURE(context, frame) endHandler(context, frame); if (context->_rescuing) handleException(context, context->_raised); break
// End the handler first so that if an exception is raised in the `ensure`,
// this handler will not rescue it
#define END_BEGIN(context, frame) run_else(context, frame); ensure(context, frame); break
#define END_RESCUE(context, frame) ensure(context, frame); break
#define END_ELSE(context, frame) ensure(context, frame); break
#define ELSE_JUMP -2
#define FRAME_RETURN -3

// Now that the ensure has been run, don't let it catch any other exceptions
// or run the ensure again
#define ensure(context, e) \
  if ( e -> ensure ) {\
    e -> in_ensure = 1;\
    e -> already_rescued = 1;\
    if (! setjmp(context->_handler_env)) \
      longjmp(e -> env, ENSURE_JUMP);}\
  else\
    endHandler(context, e);

#define run_else(context, e) \
  if ( e -> else_block ) \
    if (! setjmp(context->_handler_env)) \
      longjmp(e -> env, ELSE_JUMP);


struct Exception * EXCEPTION(object exception_class, int jump_location);

struct Exception * catchesException(exception_frame frame, object exception);

// Handle a raised exception object
void handleException(struct IridiumContext * context, object exception);

exception_frame ExceptionHandler(struct IridiumContext * context, struct list * exceptions, int ensure, int else_block, int count);

// Used for returns that occur in begin..end blocks
// Usage:
//  // begin
//  // ...
//  return_in_begin_block();
//  return value;

void return_in_begin_block(struct IridiumContext * context, object return_value);

void endHandler(struct IridiumContext * context, exception_frame e);

void IR_PUTS(struct IridiumContext * context, object obj);

// Define a constant with name (atom)
void define_constant(object name, object constant);

// Display a stacktrace
void display_stacktrace(struct IridiumContext * context, object exception);

// Lookup a constant with name (atom)
object lookup_constant(struct IridiumContext * context, object name);

// Creates the objects defined here
void IR_init_Object(struct IridiumContext * context);

#endif

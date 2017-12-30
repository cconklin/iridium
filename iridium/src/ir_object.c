#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <setjmp.h>
#include <stdarg.h>

#include "ir_object.h"

struct list * _ARGLIST(int unused, ...) {
  va_list args;
  struct list * l = NULL;
  struct array * ary = array_new();
  void * elem;
  va_start(args, unused);
  // Doing this to reverse the list
  while ((elem = va_arg(args, void *))) {
    array_push(ary, elem);
  }
  va_end(args);
  while ((elem = array_pop(ary))) {
    l = list_cons(l, elem);
  }
  return l;
}

object nil = NULL;
struct dict * constants = NULL;

struct IridiumArgument * argument_new(object name, object default_value, char splat) {
  struct IridiumArgument * arg = (struct IridiumArgument *) GC_MALLOC(sizeof(struct IridiumArgument));
  assert(arg);
  arg -> name = name;
  arg -> default_value = default_value;
  arg -> splat = splat;
  return arg;
}

// :self atom
object _SELF_ATOM = NULL;

// Function to determine object inheritance

int kindOf(object obj, object class) {
  object obj_class = obj -> class;
  while (1) {
    if (obj_class == class) {
      return 1;
    }
    if (obj_class == superclass(obj_class)) {
      break;
    }
    obj_class = superclass(obj_class);
  }
  return 0;
}

// instance_attribute lookup
// Looks for an instance attribute of a Class, looking at included modules and superclasses, NULL on failure
// Helper function of attribute_lookup

iridium_attribute
instance_attribute_lookup(object receiver, void * attribute, unsigned char access) {
  // Try and get it from the objects instance attributes
  iridium_attribute result = dict_get(receiver -> instance_attributes, attribute);
  struct list * modules;
  object module;
  
  if (no_result) {
    // Attribute was not found
    // Now look at any modules the object included
    modules = receiver -> included_modules;
    if (modules_present(modules)) {
      // If there are any included modules
      // Search all the modules until the attribute is found
      while (modules && no_result) {
        module = list_head(modules);
        result = instance_attribute_lookup(module, attribute, access);
        modules = list_tail(modules);
      }
    }
  }
  // Catches the case where result is not NULL, but has the wrong access level
  if (no_result) return NULL;
  return result;
}

// get_attribute
// Returns the value of an object attribute, nil on failure

object
get_attribute(object receiver, void * attribute, unsigned char access) {
  iridium_attribute result = attribute_lookup(receiver, attribute, access);
  return (result && result -> real) ? result -> value : NULL;
}

// Helper function
iridium_attribute
attribute_lookup(object receiver, void * attribute, unsigned char access) {
  // Look for attribute among the objects attributes
  iridium_attribute result = dict_get(receiver -> attributes, attribute);
  struct list * modules;
  object module;
  object class;
  
  if (no_result) {
    // Attribute was not found
    // Now look at any modules the object extended
    modules = receiver -> extended_modules;
    if (modules_present(modules)) {
      // If there are any extended modules
      // Search all the modules until the attribute is found
      for (module = list_head(modules); list_tail(modules) && result == NULL; modules = list_tail(modules), module = list_head(modules)) {
        result = instance_attribute_lookup(module, attribute, access);
      }
    }
  }
  if (no_result) {
    // Not in the extended modules instance attributes
    // Check the included modules attributes
    modules = receiver -> included_modules;
    if (modules_present(modules)) {
      // If there are any included modules
      // Search all the modules until the attribute is found
      for (module = list_head(modules); list_tail(modules) && no_result; modules = list_tail(modules), module = list_head(modules)) {
        result = attribute_lookup(module, attribute, access);
      }
    }
  }
  // If it is not in a module and the receiver is a class, look up the superclass chain
  if (no_result && isA(receiver, CLASS(Class))) {
    // Store the receiver in a temp so that it is not modified
    class = receiver;
    // Look up each superclass
    while (no_result && class != superclass(class)) {
      class = superclass(class);
      // Look in the superclass for the attribute
      result = attribute_lookup(class, attribute, access);
    }
  }
  if (no_result) {
    // None of the  modules had the attribute, check the class chain
    class = receiver -> class;
    // instance_attribute_lookup also checks for the classes included modules
    result = instance_attribute_lookup(class, attribute, access);
    // Check the superclass
    // Note: superclass is a normal attribute -- it undergoes attribute lookup
    // Ensure that all classes have a superclass, or this will result in an infinite loop
    // Keep searching until the attribute is found, or the class chain is at the end
    while (no_result && class != superclass(class)) {
      class = superclass(class);
      result = instance_attribute_lookup(class, attribute, access);
    }
  }
  // Catches the case where result is not NULL, but has the wrong access level
  if (no_result) return NULL;
  return result;
}

// set_attribute
// mutates the receiver, setting the attribute with `access` to a new value
object set_attribute(object receiver, void * attribute, unsigned char access, object value) {
  iridium_attribute attr;

  attr = (iridium_attribute) GC_MALLOC(sizeof(struct IridiumAttribute));
  assert(attr);
  attr -> access = access;
  attr -> real = 1;
  // set the new value
  attr -> value = (void *) value;
  dict_set(receiver -> attributes, attribute, attr);
  // Owner info for functions
  if (value -> class == CLASS(Function)) {
    internal_set_attribute(value, ATOM("owner"), receiver);
    internal_set_integral(value, ATOM("owner_type"), 0); // 0: attribute of object
  }
  return value;
}

// remove an attribute from an object (make it so that the object no longer sees an attribute)
object no_attribute(object receiver, void * attribute) {
  iridium_attribute attr;

  attr = (iridium_attribute) GC_MALLOC(sizeof(struct IridiumAttribute));
  assert(attr);
  attr -> access = 0;
  attr -> real = 0;
  // set the new value
  attr -> value = NIL;
  dict_set(receiver -> attributes, attribute, attr);
  return NIL;
}

// remove an instance attribute from an object (make it so that the object's instances no longer sees an attribute)
object no_instance_attribute(object receiver, void * attribute) {
  iridium_attribute attr;

  attr = (iridium_attribute) GC_MALLOC(sizeof(struct IridiumAttribute));
  assert(attr);
  attr -> access = 0;
  attr -> real = 0;
  // set the new value
  attr -> value = NIL;
  dict_set(receiver -> instance_attributes, attribute, attr);
  return NIL;
}

// set_instance_attribute
// mutates the receiver, setting the instance attribute with access to a new value
object set_instance_attribute(object receiver, void * attribute, unsigned char access, object value) {
  iridium_attribute attr;

  attr = (iridium_attribute) GC_MALLOC(sizeof(struct IridiumAttribute));
  assert(attr);
  attr -> access = access;
  attr -> real = 1;
  // set the new value
  attr -> value = (void *) value;
  dict_set(receiver -> instance_attributes, attribute, attr);
  // Owner info for functions
  if (value -> class == CLASS(Function)) {
    internal_set_attribute(value, ATOM("owner"), receiver);
    internal_set_integral(value, ATOM("owner_type"), 1); // 0: instance attribute of object
  }
  return value;
}

// Locals
object _local(struct dict * locals, object atm) {
    object value = dict_get(locals, atm);
    object attribute;
    struct dict * vars;
    if (value == NULL) {
        value = dict_get(locals, ATOM("self"));
        if (value == NULL) {
            // NameError
            handleException(send(CLASS(NameError), "new", IR_STRING("self")));
        } else {
            attribute = get_attribute(value, atm, PRIVATE);
            if (attribute == NULL) {
                // NameError
                handleException(send(CLASS(NameError), "new", invoke(atm, "to_s", array_new())));
            }
            // If the attribute is found, is it a function?
            if (isA(attribute, CLASS(Function))) {
              vars = internal_get_attribute(attribute, ATOM("bindings"), struct dict *);
              // The function better have some bindings, even if it is an empty dictionary.
              assert(vars);
              attribute = function_bind(attribute, dict_with(vars, ATOM("self"), value));
            }
            value = attribute;
        }
    }
    return value;
}

void _set_local(struct dict * locals, object name, object value) {
  dict_set(locals, name, value);
}

// function_bind
// Bind locals to functions
// Does not mutate the receiver
// accepts: func (object, Iridium Function), locals (struct dict *)
// returns: object (Iridirum Function)
object
function_bind(object func, struct dict * locals) {
  // attributes of Iridium functions
  object name;
  // Function arguments
  struct list * args;
  // Pointer to C function
  object (* c_func)(struct dict *);
  // Args
  args = internal_get_attribute(func, ATOM("args"), struct list *);
  // name of function
  name = get_attribute(func, ATOM("name"), PUBLIC);

  // Set the function
  c_func = internal_get_attribute(func, ATOM("function"), __typeof__(c_func));
  assert(c_func);

  // Return the new bound function
  object new_func = FUNCTION(name, args, locals, c_func);
  // retain ownership info
  internal_set_attribute(new_func, ATOM("owner"), internal_get_attribute(func, ATOM("owner"), object));
  internal_set_integral(new_func, ATOM("owner_type"), internal_get_integral(func, ATOM("owner_type"), int));
  return new_func;
}

// Unified construction sequence of objects (used in ALL constructors)

object construct(object class) {
  // Container for the object under construction
  object obj;

  // Allocate memory for the object
  obj = (object) GC_MALLOC(sizeof(struct IridiumObject));

  // Ensure that memory was allocated
  assert(obj);

  // Set the object's magic number
  obj -> magic = MAGIC;

  // Set the object's class to self (since this is a method on an instance of
  // Class, which implies that self is a class)
  obj -> class = class;

  // Initialize the attribute dictionary
  obj -> attributes = dict_new(ObjectHashsize);

  // Initialize the instance_attribute dictionary
  obj -> instance_attributes = dict_new(ObjectHashsize);

  // Initialize the internal_attribute dictionary
  obj -> internal_attributes = dict_new(ObjectHashsize);

  return obj;
}

// Function for Function calls
// Translates to:
//    obj.name(args)
// Iridium_Object_get() will retun a bound function to the `obj` #"name" method (obj will automatically be passed)
// Invoke function with handle `name` by getting the function from the object `obj`, and bind it to `self` before passing it to the C Function for Iridium Function calls.
object invoke(object obj, char * name, struct array * args) {
  // Find the callable object
  object callable = iridium_method_name(Object, __get__)(dict_with(bind_self(obj), ATOM("name"), ATOM(name)));
  return calls(callable, args);
}

object _send(object obj, char * name, ...) {
  va_list ap;
  va_start(ap, name);
  object argument;
  struct array * args = array_new();
  while ((argument = va_arg(ap, object))) {
    args = array_push(args, argument);
  }
  va_end(ap);
  return invoke(obj, name, args);
}

// Function for generic invocations
// Used by the translator

object calls(object callable, struct array * args) {
  object get_function = get_attribute(callable, ATOM("__get__"), PUBLIC);
  object call_function;
  struct dict * bindings;
  object ( * func )(struct dict *) = internal_get_attribute(get_function, ATOM("function"), object (*)(struct dict *));

  // get_function MUST BE an Iridium Function => func != NULL
  if (func == NULL) {
    handleException(send(CLASS(TypeError), "new", IR_STRING("__get__ must be a Function")));
  }

  // invoke the get_function to get the call_function
  call_function = func(dict_with(bind_self(callable), ATOM("name"), ATOM("__call__")));

  if (!isA(call_function, CLASS(Function))) {
    handleException(send(CLASS(TypeError), "new", IR_STRING("__call__ must be a Function")));
  }

  // call_function IS an Iridium Function
  bindings = process_args(call_function, args);
  func = internal_get_attribute(call_function, ATOM("function"), object (*)(struct dict *));
  assert(func);
  return func(bindings);
}

// Converts iridium strings to C strings
char * str(object string) {
  return internal_get_attribute(string, ATOM("string"), char *);
}

// Destructures arrays into struct array *
struct array * destructure(struct array * args, object argument_array) {
  struct array * array_args = internal_get_attribute(argument_array, ATOM("array"), struct array *);
  object reason;
  if (array_args == NULL) {
    // Not actually an array...
    reason = IR_STRING("Cannot destructure ");
    reason = send(reason, "__add__", _send(argument_array, "to_s", 0));
    reason = send(reason, "__add__", IR_STRING(":"));
    reason = send(reason, "__add__", _send(argument_array -> class, "to_s", 0));
    handleException(send(CLASS(TypeError), "new", reason));
  }
  return array_merge(args, array_args);
}

// Create a binding dictionary by processing function arguments (retrieve closed values and arguments)
struct dict * process_args(object function, struct array * _args) {
  struct array * args = array_copy(_args);
  struct dict * closed_values = internal_get_attribute(function, ATOM("bindings"), struct dict *);
  struct dict * argument_values = dict_new(ObjectHashsize);
  struct list * argument_list = internal_get_attribute(function, ATOM("args"), struct list *);
  struct list * iter_arg_list = argument_list;
  struct array * partial_args;
  int splat = 0;
  int first_optional = -1;
  int index = 0;
  int splat_idx = 0;
  int length = 0;
  int arg_length = args -> length;
  unsigned int required = 0;
  struct IridiumArgument * this_arg;
  object reason;
  while (iter_arg_list) {
    this_arg = (struct IridiumArgument *) list_head(iter_arg_list);
    if (this_arg -> splat) {
      splat ++;
    }
    // Not yet found an optional argument and argument is optional
    if ((first_optional < 0) && (this_arg -> default_value != NULL)) {
      first_optional = index;
    }
    // Required argument
    if ((this_arg -> default_value == NULL) && (this_arg -> splat == 0)) {
      required ++;
    }
    // There is a required argument after an optional one
    if ((first_optional >= 0) && (this_arg -> default_value == NULL) && (this_arg -> splat == 0)) {
      reason = IR_STRING("Cannot have a required argument after an optional one (");
      reason = send(reason, "__add__", _send(function, "to_s", 0));
      reason = send(reason, "__add__", IR_STRING(")"));
      handleException(send(CLASS(ArgumentError), "new", reason));
    }
    iter_arg_list = list_tail(iter_arg_list);
    index ++;
  }
  length = index;
  // ensure that the argument list does not have multiple splatted args
  if (splat >= 2) {
    reason = IR_STRING("Cannot have multiple splatted args (ex. *args) (");
    reason = send(reason, "__add__", _send(function, "to_s", 0));
    reason = send(reason, "__add__", IR_STRING(")"));
    handleException(send(CLASS(ArgumentError), "new", reason));
  }
  // ensure that the arity is sufficient
  // if there is no splat, ensure that the arity is not too large
  if ((arg_length < required) || ((splat == 0) && (arg_length > length))) {
    reason = IR_STRING("Wrong number of arguments: ");
    reason = send(reason, "__add__", _send(FIXNUM(arg_length), "to_s", 0));
    reason = send(reason, "__add__", IR_STRING(" for "));
    reason = send(reason, "__add__", _send(FIXNUM(required), "to_s", 0));
    if (required != length) {
      reason = send(reason, "__add__", IR_STRING(".."));
      reason = send(reason, "__add__", _send(FIXNUM(length), "to_s", 0));
    }
    reason = send(reason, "__add__", IR_STRING(" ("));
    reason = send(reason, "__add__", _send(function, "to_s", 0));
    reason = send(reason, "__add__", IR_STRING(")"));
    handleException(send(CLASS(ArgumentError), "new", reason));
  }
  
  // function x(a, * args, b = 2, c = 4) ...
  // x(1) # > a = 1, args = {}, b = 2, c = 4
  // x(1, 3) # > a = 1, args = {}, b = 3, c = 4
  // x(1, 1, 3) # > a = 1, args = {}, b = 1, c = 3
  // x(1, 1, 3, 5) # > a = 1, args = {1}, b = 3, c = 5

  iter_arg_list = argument_list;
  index = 0;
  // array for destructured arg (if present)
  partial_args = array_new();
  while (iter_arg_list) {
    this_arg = (struct IridiumArgument *) list_head(iter_arg_list);
    // required argument or optional one (not splatted)
    if (this_arg -> splat == 0) {
      if (index < arg_length) {
        dict_set(argument_values, this_arg -> name, array_shift(args));          
      } else {
        // All of the passed args have been used
        // Fill in the default values for the optional arguments
        dict_set(argument_values, this_arg -> name, this_arg -> default_value);          
      }
      index ++;  
    } else {
      splat_idx = 0;
      while (splat_idx < (arg_length - (length - splat))) {
        array_push(partial_args, array_shift(args));
        splat_idx ++;
        index ++;
      }
      // Splatted argument
      dict_set(argument_values, this_arg -> name, ARRAY(partial_args));
    }
    iter_arg_list = list_tail(iter_arg_list);
  } 
  return dict_merge(closed_values, argument_values);
}


// class Class

// Class.new
// Constructor of classes
// Locals used: self, superclass, args
// Output:      obj (new class)
iridium_classmethod(Class, new) {
  // Receiver
  object self = local("self");
  // Superclass
  object superclass = local("superclass");
  // Class name
  object name = local("name");
  // Any args for initialize
  object args = local("args");
  // Create the class
  object obj = construct(self);
  // Set the superclass (needed for invoke to work)
  set_attribute(obj, ATOM("superclass"), PUBLIC, superclass);
  // Set the name
  set_attribute(obj, ATOM("name"), PUBLIC, name);
  // Call initialize, merging the superclass with any other args
  invoke(obj, "initialize", destructure(array_push(array_push(array_new(), name), superclass), args));
  // Return the created object
  return obj;
}

// Class#new
// Constructor of objects
// Inputs: locals (struct dict *, dictionary of local variables used in closures)
//            Locals used: self, args
// Output:  obj (object)

// function new(* args)
iridium_method(Class, new) {

  // Value of the receiver
  object self = local("self");
  // Value of the args
  object args = local("args"); // array
  
  // Container for the object under construction
  object obj = construct(self);
  // Send off to the object initialize method
  invoke(obj, "initialize", destructure(array_new(), args));
  
  // Now that the object is constructed and initialized, return it
  return obj;
  
}

// Class#inspect
// Converts a class into an iridium string
// Locals used: self
// Output: Iridium String
iridium_method(Class, inspect) {
  // Receiver
  object self = local("self");
  object str = get_attribute(self, ATOM("name"), PUBLIC);
  return str;
}

// class Object

// Object#class
// Returns an object's class
iridium_method(Object, class) {
  object self = local("self");
  return self -> class;
}

// Object#puts
// Displays to stdout
// Locals used: args
// Output: nil
iridium_method(Object, puts) {
  int idx = 0;
  struct array * obj_ary = internal_get_attribute(local("args"), ATOM("array"), struct array *);
  object * objs = (object *) obj_ary -> elements;
  int end = obj_ary -> length;
  for (idx = 0; idx < end-1; idx ++) {
    printf("%s ", C_STRING(send(objs[idx], "to_s")));
  }
  if (end-1>=0) {
    printf("%s", C_STRING(send(objs[end-1], "to_s")));
  }
  printf("\n");
  return NIL;
}

// Object#write
// Displays to stdout without ending newline
// Locals used: args
// Output: nil
iridium_method(Object, write) {
  int idx = 0;
  struct array * obj_ary = internal_get_attribute(local("args"), ATOM("array"), struct array *);
  object * objs = (object *) obj_ary -> elements;
  int end = obj_ary -> length;
  for (idx = 0; idx < end-1; idx ++) {
    printf("%s ", C_STRING(send(objs[idx], "to_s")));
  }
  if (end-1>=0) {
    printf("%s", C_STRING(send(objs[end-1], "to_s")));
  }
  return NIL;
}



// Object#gets
iridium_method(Object, gets) {
  char * gets_buffer;
  char * str_buffer;
  size_t gets_len;
  object prompt = local("prompt");
  printf("%s", C_STRING(prompt));
  getline(&gets_buffer, &gets_len, stdin);
  str_buffer = GC_MALLOC((strlen(gets_buffer))*sizeof(char));
  gets_buffer[strlen(gets_buffer)-1] = 0; // Remove the newline
  assert(str_buffer);
  strcpy(str_buffer, gets_buffer);
  free(gets_buffer); // Not allocated by the GC -- need to free it manually
  return IR_STRING(str_buffer);
}

// Object#raise
// Raise an exception
// Locals used: exc
iridium_method(Object, raise) {
  handleException(local("exc"));
  // Shouldn't get here
  return NIL;
}

// Object#__eq__
// Returns true if objects are equal, false otherwise
iridium_method(Object, __eq__) {
  return (local("self") == local("other")) ? ir_cmp_true : ir_cmp_false;
}

// Object#__neq__
// Returns true if objects are not equal, false otherwise
iridium_method(Object, __neq__) {
  return (local("self") != local("other")) ? ir_cmp_true : ir_cmp_false;
}


// Object#inspect
// Converts an object into an iridium string
// Locals used: self
// Output: Iridium String
iridium_method(Object, inspect) {
  // Receiver
  object self = local("self");
  object class_name = get_attribute(self->class, ATOM("name"), PUBLIC);
  char buffer[1000];
  char * c_str;
  sprintf(buffer, "#<%s:%p>", C_STRING(class_name), self);
  c_str = GC_MALLOC((strlen(buffer) + 1) * sizeof(char));
  assert(c_str);
  strcpy(c_str, buffer);
  return IR_STRING(c_str);
}


// Object#to_s
// Converts an object into an iridium string
// Locals used: self
// Output: Iridium String
iridium_method(Object, to_s) {
  // Receiver
  object self = local("self");
  return invoke(self, "inspect", array_new());
}

// Object#__get__
// Gets attributes of objects, looking up the object hierarchy
// Inputs:  self (object, any arbitrary Iridium object)
//          locals (struct dict *, dictionary of local variables used in closures)
//            Locals used: self
//          args
//            Arity: 1
//            name
// Output:  obj (object)
// Iridium Example: obj.get(:value) # => :some_value

iridium_method(Object, __get__) {

  // Value of the receiver
  object self = local("self");
  
  // Dictionary of attribute locals, if attribute is a function
  struct dict * vars;

  // Grab the key from the locals
  object name = local("name");
  
  // Look for the attribute
  object attribute = get_attribute(self, name, PUBLIC);

  // If the attribute is NULL: raise an exception
  if (NULL == attribute) {
    object reason = send(name, "to_s");
    reason = send(reason, "__add__", IR_STRING(" is not defined on "));
    reason = send(reason, "__add__", invoke(self, "inspect", array_new()));
    reason = send(reason, "__add__", IR_STRING(":"));
    reason = send(reason, "__add__", invoke(self->class, "inspect", array_new()));
    handleException(send(CLASS(AttributeError), "new", reason));
  }

  // If the attribute is found, is it a function?
  if (isA(attribute, CLASS(Function))) {
    vars = internal_get_attribute(attribute, ATOM("bindings"), struct dict *);
    // The function better have some bindings, even if it is an empty dictionary.
    assert(vars);
    attribute = function_bind(attribute, dict_with(vars, ATOM("self"), self));
  }

  return attribute;
}

// Object#has_attribute?
// Returns true if object has the named attribute, false otherwise
// Inputs:  self, attr (atom)
// Outputs: Boolean
iridium_method(Object, has_attribute) {
  object self = local("self");
  object attr = local("attr");
  object attribute = get_attribute(self, attr, PUBLIC);
  return (attribute == NULL) ? ir_cmp_false : ir_cmp_true;
}

// Object#__set__
// Sets attributes of objects. Always sets the attribute on the receiver,
// not anywhere else in the object hierarchy.
// Inputs:  self (object, any arbitrary Iridium object)
//          locals (struct dict *, dictionary of local variables used in closures)
//            Locals used: self
//          args
//            Arity: 2
//            name
//            value
// Output:  obj (object, the value of the passed attribute)
// Iridium Example: obj.set(:value, :some_value) # => :some_value

iridium_method(Object, __set__) {

  // Value of the receiver
  object self = local("self");

  // Grab the key from the argument array
  object name = local("name");

  // Get the attribute from the argument array
  object value = local("value");

  // Set the attribute on the object as a public attribute
  set_attribute(self, name, PUBLIC, value);

  return value;
}

// Object#initialize
// Default initilization behavior for objects.
// Inputs:  self (object, any arbitrary Iridium object)
//          locals (struct dict *, dictionary of local variables used in closures)
//            Locals used:
//          args (array of objects)
//            Arity: 0
// Output:  nil
// Iridium Example: obj.initialize() # => nil

iridium_method(Object, initialize) {
  return NIL;
}

// class Module

iridium_method(Module, include) {
  object self = local("self");
  object mod = local("module");
  if (mod -> class != CLASS(Module)) {
    // Not including a module -- bad!
    object reason = send(mod, "to_s");
    reason = send(reason, "__add__", IR_STRING(" is not a module"));
    handleException(send(CLASS(TypeError), "new", reason));
  } else {
    self -> included_modules = list_cons(self -> included_modules, mod);
  }
  return NIL;
}

iridium_method(Module, define_method) {
  object self = local("self");
  object name = local("name");
  object method = local("fn");
  object name_reason = send(name, "inspect");
  name_reason = send(name_reason, "__add__", IR_STRING(" is not an atom"));
  object fn_reason = send(method, "inspect");
  fn_reason = send(fn_reason, "__add__", IR_STRING(" is not a function"));
  if (method -> class != CLASS(Function)) {
    handleException(send(CLASS(TypeError), "new", fn_reason));
  }
  if (name -> class != CLASS(Atom)) {
    handleException(send(CLASS(TypeError), "new", name_reason));
  }
  set_instance_attribute(self, name, PUBLIC, method);
  return name;
}

// class Function

// Function#__call__
// Invokes a function
// method __call__(* args)
iridium_method(Function, __call__) {
  // Locals to be passed to the C function
  struct dict * bindings;
  // Value of the receiver
  object self = local("self");
  // Get args to be passed to function
  object args = local("args"); // array
  
  // Get the corresponding C function
  object ( * func )(struct dict *) = internal_get_attribute(self, ATOM("function"), object (*)(struct dict *));
  assert(func);

  // Set the locals to that of the function (retrieve closed values and arguments)
  bindings = process_args(self, destructure(array_new(), args));
      
  // Invoke the function
  return func(dict_copy(bindings));
}

// FUNCTION helper
// args:
//  name (Atom)
//  args (struct list * (of arguments))
//  bindings (struct dict *)
//  func (iridium_method)
//  Convert a C function to an Iridium Function
object FUNCTION(object name, struct list * args, struct dict * bindings, object (* func)(struct dict *)) {
  object self = construct(CLASS(Function));
  internal_set_attribute(self, ATOM("args"), args);
  set_attribute(self, ATOM("name"), PUBLIC, name);
  internal_set_attribute(self, ATOM("bindings"), dict_copy(bindings));
  internal_set_attribute(self, ATOM("function"), func);
  return self;
}

iridium_method(Function, inspect) {
  object self = local("self");
  object name = get_attribute(self, ATOM("name"), PUBLIC);
  char * given_name = internal_get_attribute(name, ATOM("string"), char *);
  char buffer[100];
  object owner = internal_get_attribute(self, ATOM("owner"), object);
  // 1: instance attribute of owner, 0: attribute of owner
  int owner_type = internal_get_integral(self, ATOM("owner_type"), int);
  if (owner == self) {
    // Avoid to_s'ing ourself forever
    sprintf(buffer, "#<Function self.%s:%p>", given_name, self);
  } else if (owner) {
    sprintf(buffer, "#<Function %s%c%s:%p>", C_STRING(invoke(owner, "inspect", array_new())), (owner_type ? '#' : '.'), given_name, self);
  } else {
    sprintf(buffer, "#<Function %s:%p>", given_name, self);
  }
  char * full_name = GC_MALLOC((strlen(buffer) + 1) * sizeof(char));
  strcpy(full_name, buffer);
  return IR_STRING(full_name);
}

// class Atom

// Atom.new
// Creates an atom
// Arity: 1
//  name (Iridium String)
void * ATOM_TABLE_KEY = NULL;

// function new(name)
iridium_classmethod(Atom, new) {
  object atom;
  // Get the name of the atom
  object name = local("name");
  struct array * args = array_new();

  // Create the atom
  // TODO define str which converts Iridium strings to C strings
  atom = ATOM(str(name));

  array_set(args, 0, name);
  // Initialize the atom (unless overloaded in an Iridium Program, will default to doing nothing at all)
  invoke(atom, "initialize", args);

  return atom;
}

struct dict * ATOM_TABLE() {
  // Atom class
  object atom_class = CLASS(Atom);

  // Atom table
  struct dict * atom_table;

  // Ensure that the atom table exists
  if (! ATOM_TABLE_KEY) {
    // Get a piece of memory that is guaranteed to be unique
    ATOM_TABLE_KEY = malloc(sizeof(char));
    assert(ATOM_TABLE_KEY);
    atom_table = str_dict_new(ATOM_HASHSIZE);
    // The atom table is a dictionary with C string keys (most dicts use C pointers)
    internal_set_attribute(atom_class, ATOM_TABLE_KEY, atom_table);
    return atom_table;
  } else {
    // The atom table exists, return it
    return internal_get_attribute(atom_class, ATOM_TABLE_KEY, struct dict *);
  }

}

void add_atom(char * name, object atom) {
  str_dict_set(ATOM_TABLE(), name, atom);
}

// Create the `:string` atom
object create_str_atom() {
  // Allocate memory for the object
  object str_atom = construct(CLASS(Atom));
  // Set atom name
  internal_set_attribute(str_atom, str_atom, "string");

  // Add it to the Atom table
  add_atom("string", str_atom);

  // Return the atom
  return str_atom;
}

// Create the `:self` atom
object create_self_atom() {
  // Allocate memory for the object
  object self_atom = construct(CLASS(Atom));
  // Set atom name
  internal_set_attribute(self_atom, ATOM("string"), "self");

  // Add it to the Atom table
  add_atom("self", self_atom);

  // Set _SELF_ATOM
  _SELF_ATOM = self_atom;

  // Return the atom
  return self_atom;
}

// Create or fetch atoms
object _ATOM(char * name) {
  // Container for the new atom
  object atom;


  // Table of atoms
  // Note: The key for the table is the only dict key that is not an atom.
  struct dict * atom_table = ATOM_TABLE();

  if (! (atom = (object) str_dict_get(atom_table, name))) {
    // The atom does not exist
    // Create the new object
    atom = construct(CLASS(Atom));

    // Add it to the Atom table
    add_atom(name, atom);

    // Set atom name
    internal_set_attribute(atom, ATOM("string"), name);

  }
  return atom;
}

iridium_method(Atom, inspect) {
  object self = local("self");
  char buffer[100];
  sprintf(buffer, ":%s", internal_get_attribute(self, ATOM("string"), char *));
  char * str = GC_MALLOC((strlen(buffer + 1) * sizeof(char)));
  assert(str);
  strcpy(str, buffer);
  return IR_STRING(str);
}

iridium_method(Atom, to_s) {
  object self = local("self");
  char * buffer = internal_get_attribute(self, ATOM("string"), char *);
  char * str = GC_MALLOC((strlen(buffer + 1) * sizeof(char)));
  assert(str);
  strcpy(str, buffer);
  return IR_STRING(str);
}

// class Array

object ARRAY(struct array * values) {
  object array = construct(CLASS(Array));
  internal_set_attribute(array, ATOM("array"), values);
  return array;
}

iridium_classmethod(Array, new) {
  object args = local("args");
  object array = ARRAY(destructure(array_new(), args));
  // Call initialize (which should do nothing unless overidden)
  invoke(array, "initialize", destructure(array_new(), args));
  return array;
}

// method reduce(accumulator, fn)
iridium_method(Array, reduce) {
  object self = local("self");
  object accumulator = local("accumulator");
  object fn = local("fn");
  object element;
  // Get a duplicate of the internal array
  struct array * ary = array_copy(internal_get_attribute(self, ATOM("array"), struct array *));
  struct array * args;
  while (ary -> length) {
    element = array_shift(ary);
    args = array_new();
    array_set(args, 0, element);
    array_set(args, 1, accumulator);
    accumulator = calls(fn, args);
  }
  return accumulator;
}

iridium_method(Array, __get_index__) {
  object self = local("self");
  object index = local("index"); // Iridium Integer
  struct array * ary = internal_get_attribute(self, ATOM("array"), struct array *);
  object result = array_get(ary, INT(index));
  if (result) {
    return result;
  } else {
    return NIL;
  }
}

iridium_method(Array, __set_index__) {
  object self = local("self");
  object index = local("index"); // Iridium Integer
  object value = local("value");
  struct array * ary = internal_get_attribute(self, ATOM("array"), struct array *);
  array_set(ary, INT(index), value);
  return value;
}

iridium_method(Array, inspect) {
  object self = local("self");
  struct array * ary = internal_get_attribute(self, ATOM("array"), struct array *);
  int idx, sidx;
  unsigned int strsize = 0;
  unsigned ary_len = (ary->length - ary->start);
  if (ary_len == 0) {
    return IR_STRING("[]");
  }
  char ** strs = malloc(ary_len*sizeof(char *));
  char * str;
  assert(strs);
  for (idx = ary -> start; idx < ary -> length; idx++) {
    sidx = idx - (ary -> start);
    strs[sidx] = C_STRING(invoke(array_get(ary, idx), "inspect", array_new()));
    strsize += strlen(strs[sidx]);
  }
  if (ary_len > 1) {
    strsize += (ary_len-1) * 2; // A space and comma for each entry (except for the last)
  }
  strsize += 2; // opening and closing brackets
  strsize++; // Account for the terminating NULL
  str = GC_MALLOC(strsize*sizeof(char));
  assert(str);
  str[0] = '[';
  str[1] = 0;
  for (idx = 0; idx < ary_len-1; idx++) {
    strcat(str, strs[idx]);
    strcat(str, ", ");
  }
  strcat(str, strs[ary_len-1]);
  strcat(str, "]");
  free(strs);
  return IR_STRING(str);
}

iridium_method(Array, push) {
  object self = local("self");
  object value = local("value");
  struct array * ary = internal_get_attribute(self, ATOM("array"), struct array *);
  array_push(ary, value);
  return self;
}

iridium_method(Array, unshift) {
  object self = local("self");
  object value = local("value");
  struct array * ary = internal_get_attribute(self, ATOM("array"), struct array *);
  ary = array_unshift(ary, value);
  internal_set_attribute(self, ATOM("array"), ary);
  return self;
}

// class Boolean

iridium_classmethod(true, inspect) {
    return IR_STRING("true");
}

iridium_classmethod(false, inspect) {
    return IR_STRING("false");
}

// Class AttributeError

iridium_method(AttributeError, initialize) {
    object self = local("self");
    object message = local("message");
    send(self, "__set__", ATOM("message"), message);
    return NIL;
}

iridium_method(AttributeError, reason) {
    object self = local("self");
    object message = send(self, "__get__", ATOM("message"));
    return message;
}

// class Integer

iridium_classmethod(Integer, new) {
  object fixnum = local("fixnum");
  return fixnum;
}

iridium_method(Integer, __add__) {
  object self = local("self");
  object other = local("other");
  return FIXNUM(INT(self) + INT(other));
}

iridium_method(Integer, __sub__) {
  object self = local("self");
  object other = local("other");
  return FIXNUM(INT(self) - INT(other));
}

iridium_method(Integer, __eq__) {
  object self = local("self");
  object other = local("other");
  if (self -> class != other -> class) {
    return ir_cmp_false;
  } else if (INT(self) == INT(other)) {
    return ir_cmp_true;
  } else {
    return ir_cmp_false;
  }
}

iridium_method(Integer, __lt__) {
  object self = local("self");
  object other = local("other");
  if (self -> class != other -> class) {
    return ir_cmp_false;
  } else if (INT(self) < INT(other)) {
    return ir_cmp_true;
  } else {
    return ir_cmp_false;
  }
}

iridium_method(Integer, __gt__) {
  object self = local("self");
  object other = local("other");
  if (self -> class != other -> class) {
    return ir_cmp_false;
  } else if (INT(self) > INT(other)) {
    return ir_cmp_true;
  } else {
    return ir_cmp_false;
  }
}

iridium_method(Integer, __leq__) {
  object self = local("self");
  object other = local("other");
  if (self -> class != other -> class) {
    return ir_cmp_false;
  } else if (INT(self) <= INT(other)) {
    return ir_cmp_true;
  } else {
    return ir_cmp_false;
  }
}

iridium_method(Integer, __geq__) {
  object self = local("self");
  object other = local("other");
  if (self -> class != other -> class) {
    return ir_cmp_false;
  } else if (INT(self) >= INT(other)) {
    return ir_cmp_true;
  } else {
    return ir_cmp_false;
  }
}

iridium_method(Integer, __neq__) {
  object self = local("self");
  object other = local("other");
  if (send(self, "__eq__", other) == ir_cmp_false) {
    return ir_cmp_true;
  } else {
    return ir_cmp_false;
  }
}


object FIXNUM(int val) {
  object fixnum = construct(CLASS(Integer));
  
  internal_set_integral(fixnum, ATOM("value"), val);
  return fixnum;
}

int INT(object fixnum) {
  return internal_get_integral(fixnum, ATOM("value"), int);
}

iridium_method(Integer, inspect) {
  object self = local("self");
  int val = INT(self);
  char buffer[30];
  sprintf(buffer, "%d", val);
  char * str = GC_MALLOC((strlen(buffer) + 1) * sizeof(char));
  strcpy(str, buffer);
  return IR_STRING(str);
}

// class String

object IR_STRING(char * c_str) {
  object str = construct(CLASS(String));
  internal_set_attribute(str, ATOM("str"), c_str);
  return str;
}

char * C_STRING(object str) {
  object reason;
  if (str -> class != CLASS(String)) {
    reason = send(str, "inspect");
    reason = send(reason, "__add__", IR_STRING(" is not a string object"));
    handleException(send(CLASS(TypeError), "new", reason));
  }
  return internal_get_attribute(str, ATOM("str"), char *);
}

// String#to_s
// Returns a copy of the same string
iridium_method(String, to_s) {
  object self = local("self");
  char * str = C_STRING(self);
  unsigned int l = strlen(str);
  char * str_copy = GC_MALLOC((l+1)*sizeof(char));
  strcpy(str_copy, str);
  return IR_STRING(str_copy);
}

// String#inspect
// Returns a copy of the same string with quotes
iridium_method(String, inspect) {
  object self = local("self");
  char * str = C_STRING(self);
  unsigned int l = strlen(str);
  char * str_copy = GC_MALLOC((l+3)*sizeof(char));
  str_copy[0] = '"';
  str_copy[1] = 0;
  strcat(str_copy, str);
  str_copy[l+1] = '"';
  str_copy[l+2] = 0;
  return IR_STRING(str_copy);
}

// class NilClass

object create_nil() {
  // Bail if nil is already defined
  if (nil) return nil;
  // TODO ensure that NilClass exists
  nil = construct(CLASS(NilClass));
  // TODO make most attributes dissapear
  return nil;
}

iridium_method(NilClass, inspect) {
  return IR_STRING("nil");
}

// exit()
iridium_method(Object, exit) {
  object code = local("exitstatus");
  exit(INT(code));
  return NIL;
}

// class Exception

// Global variable to indicate if an exception is being handled
int _rescuing = 0;

struct Exception * EXCEPTION(object exception_class, int jump_location) {
  struct Exception * exception = (struct Exception *) GC_MALLOC(sizeof(struct Exception));
  assert(exception);
  exception -> class = exception_class;
  exception -> jump_location = jump_location;
  return exception;
}

struct Exception * catchesException(exception_frame frame, object exception) {
  struct Exception * e;
  struct list * exceptions = frame -> exceptions;
  while ( exceptions ) {
    e = list_head(exceptions);
    if ( kindOf( exception, e -> class ) ) {
      return e;
    }
    exceptions = list_tail(exceptions);
  }
  return NULL;
}

// Handle a raised exception object
void handleException(object exception) {
  struct stack * frames = _exception_frames;
  // Active exception frame
  exception_frame frame;
  // Exception handler information
  struct Exception * e;
  // Should not be NULL
  assert(frames);
  // Ensure that the raised exception is an Object
  _raised = exception;
  assert(isObject(_raised));
  // Indicate that an exception is being handled
  _rescuing = 1;
  // Should have at least one handler
  while(! stack_empty(frames)) {
    frame = (exception_frame) stack_top(frames);
    if ((e = catchesException(frame, exception)) && !(frame -> already_rescued)) {
      // Jump to the appropriate handler
      // Binding of the exception to the variable happens during the exception handler
      // Before we jump, we need to indicate that we are no longer handling the exception
      // since a handler has been located
      _rescuing = 0;
      frame -> already_rescued = 1;
      longjmp(frame -> env, e -> jump_location);
    }
    if (!(frame -> in_ensure)) {
      // Run the ensure
      ensure(frame);
    } else {
      // There was an exception within the ensure
      // That means it never removed itself from the stack
      endHandler(frame);
    }
  }
  // Should NEVER get here
}

exception_frame ExceptionHandler(struct list * exceptions, int ensure, int else_block, int count) {
  exception_frame frame = (exception_frame) GC_MALLOC(sizeof(struct ExceptionFrame));
  assert(frame);
  frame -> exceptions = exceptions;
  frame -> ensure = ensure;
  frame -> count = count;
  frame -> else_block = else_block;
  frame -> already_rescued = 0;
  frame -> in_ensure = 0;
  frame -> return_value = NULL;
  // Push to the exception handler stack
  stack_push(_exception_frames, frame);
  return frame;
}

// Used for returns that occur in begin..end blocks
// Usage:
//  // begin
//  // ...
//  return_in_begin_block();
//  return value;

void return_in_begin_block(object return_value) {
  struct stack * frames = _exception_frames;
  exception_frame frame;
  assert(frames);
  // Should have at least one handler
  while(! stack_empty(frames)) {
    frame = (exception_frame) stack_top(frames);
    // Record intended return value
    frame -> return_value = return_value;
    if (!(frame -> in_ensure)) {
      // Run the ensure
      ensure(frame);
    } else {
      // There was an return within the ensure
      // That means it never removed itself from the stack
      endHandler(frame);
    }
    // This is the last frame in this iridium function
    // Don't process any more
    if (frame -> count == 0) {
      break;
    }
  }
}

void endHandler(exception_frame e) {
  // Check to ensure that this is not called incorrectly
  assert(e == stack_top(_exception_frames));
  stack_pop(_exception_frames);
  // If it showed intent at returning a value, try to return it
  if (e -> return_value) {    
    longjmp(e -> env, FRAME_RETURN);
  }
}

void IR_PUTS(object obj) {
  printf("%s\n", C_STRING(invoke(obj, "to_s", array_new())));
}

// Hash method for objects
iridium_method(Object, hash) {
  return FIXNUM((long long int) local("self"));
}

// Hash method for Integers
iridium_method(Integer, hash) {
  object self = local("self");
  long long int val = INT(self);
  return FIXNUM((val << 19) + (val >> 3));
}

// Define a constant with name (atom)
void define_constant(object name, object constant) {
  if (constants == NULL) {
    constants = dict_new(ObjectHashsize);
  }
  dict_set(constants, name, constant);
}

// Lookup a constant with name (atom)
object lookup_constant(object name) {
  object constant = (object) dict_get(constants, name);
  object reason;
  if (constant == NULL) {
    // Not a base constant, look under the current context
    constant = get_attribute(ir_context, name, PUBLIC);
    if (constant == NULL) {
      reason = send(name, "to_s");
      reason = send(reason, "__add__", IR_STRING(" is not a valid constant"));
      handleException(send(CLASS(NameError), "new", reason));
    }
  }
  return constant;
}

// Creates the objects defined here
void IR_init_Object() {
  
  struct IridiumArgument * args;
  struct IridiumArgument * name;
  struct IridiumArgument * class_superclass;
  struct IridiumArgument * class_name;
  struct IridiumArgument * other;
  object call, get, class_new, class_inst_new, obj_init, class_inspect, object_inspect, fix_plus, nil_inspect, fix_inspect, atom_inspect, func_inspect, obj_puts, str_inspect, obj_write;
  
  // Create class
  CLASS(Class) = construct(CLASS(Class));
  CLASS(Class) -> class = CLASS(Class);

  // Create Module
  CLASS(Module) = construct(CLASS(Class));
  CLASS(Module) -> class = CLASS(Class);
  
  // Create Atom
  CLASS(Atom) = construct(CLASS(Class));
  create_str_atom();
  
  // Create object
  CLASS(Object) = construct(CLASS(Class));
  set_attribute(CLASS(Atom), ATOM("superclass"), PUBLIC, CLASS(Object));
  set_attribute(CLASS(Module), ATOM("superclass"), PUBLIC, CLASS(Object));
  set_attribute(CLASS(Class), ATOM("superclass"), PUBLIC, CLASS(Module));
  set_attribute(CLASS(Object), ATOM("superclass"), PUBLIC, CLASS(Object));
  
  // Create Function
  CLASS(Function) = construct(CLASS(Class));
  set_attribute(CLASS(Function), ATOM("superclass"), PUBLIC, CLASS(Object));

  args = argument_new(ATOM("args"), NULL, 1);
  call = FUNCTION(ATOM("__call__"), list_new(args), dict_new(ObjectHashsize), iridium_method_name(Function, __call__));

  name = argument_new(ATOM("name"), NULL, 0);
  get = FUNCTION(ATOM("__get__"), list_new(name), dict_new(ObjectHashsize), iridium_method_name(Object, __get__));

  set_instance_attribute(CLASS(Function), ATOM("__call__"), PUBLIC, call);
  set_instance_attribute(CLASS(Object), ATOM("__get__"), PUBLIC, get);

  CLASS(String) = construct(CLASS(Class));
  set_attribute(CLASS(String), ATOM("superclass"), PUBLIC, CLASS(Object));

  // Bootstrap Class
  class_superclass = argument_new(ATOM("superclass"), CLASS(Object), 0);
  class_name = argument_new(ATOM("name"), NULL, 0);
  class_new = FUNCTION(ATOM("new"), list_cons(list_cons(list_new(args), class_superclass), class_name), dict_new(ObjectHashsize), iridium_classmethod_name(Class, new));
  set_attribute(CLASS(Class), ATOM("new"), PUBLIC, class_new);
  class_inst_new = FUNCTION(ATOM("new"), list_new(args), dict_new(ObjectHashsize), iridium_method_name(Class, new));
  set_instance_attribute(CLASS(Class), ATOM("new"), PUBLIC, class_inst_new);
  class_inspect = FUNCTION(ATOM("inspect"), NULL, dict_new(ObjectHashsize), iridium_method_name(Class, inspect));
  set_instance_attribute(CLASS(Module), ATOM("inspect"), PUBLIC, class_inspect);
  // Bootstrap Object
  obj_puts = FUNCTION(ATOM("puts"), ARGLIST(argument_new(ATOM("args"), NULL, 1)), dict_new(ObjectHashsize), iridium_method_name(Object, puts));
  obj_write = FUNCTION(ATOM("write"), ARGLIST(argument_new(ATOM("args"), NULL, 1)), dict_new(ObjectHashsize), iridium_method_name(Object, write));
  set_instance_attribute(CLASS(Object), ATOM("puts"), PUBLIC, obj_puts);
  set_instance_attribute(CLASS(Object), ATOM("write"), PUBLIC, obj_write);
  obj_init = FUNCTION(ATOM("initialize"), list_new(args), dict_new(ObjectHashsize), iridium_method_name(Object, initialize));
  set_instance_attribute(CLASS(Object), ATOM("initialize"), PUBLIC, obj_init);
  object_inspect = FUNCTION(ATOM("inspect"), NULL, dict_new(ObjectHashsize), iridium_method_name(Object, inspect));
  set_instance_attribute(CLASS(Object), ATOM("inspect"), PUBLIC, object_inspect);
  DEF_METHOD(CLASS(Object), "to_s", ARGLIST(), iridium_method_name(Object, to_s));
  DEF_METHOD(CLASS(Object), "__set__", ARGLIST(name, argument_new(ATOM("value"), NULL, 0)), iridium_method_name(Object, __set__));
  DEF_METHOD(CLASS(Object), "raise", ARGLIST(argument_new(ATOM("exc"), NULL, 0)), iridium_method_name(Object, raise));
  DEF_METHOD(CLASS(Object), "exit", ARGLIST(argument_new(ATOM("exitstatus"), NULL, 0)), iridium_method_name(Object, exit));
  DEF_METHOD(CLASS(Object), "class", ARGLIST(), iridium_method_name(Object, class));
  DEF_METHOD(CLASS(Object), "__eq__", ARGLIST(argument_new(ATOM("other"), NULL, 0)), iridium_method_name(Object, __eq__));
  DEF_METHOD(CLASS(Object), "__neq__", ARGLIST(argument_new(ATOM("other"), NULL, 0)), iridium_method_name(Object, __neq__));
  DEF_METHOD(CLASS(Object), "hash", ARGLIST(), iridium_method_name(Object, hash));
  DEF_METHOD(CLASS(Object), "gets", ARGLIST(argument_new(ATOM("prompt"), IR_STRING(""), 0)), iridium_method_name(Object, gets));
  DEF_METHOD(CLASS(Object), "has_attribute?", ARGLIST(argument_new(ATOM("attr"), NULL, 0)), iridium_method_name(Object, has_attribute));
  DEF_METHOD(CLASS(Module), "define_method", ARGLIST(argument_new(ATOM("name"), NULL, 0), argument_new(ATOM("fn"), NULL, 0)), iridium_method_name(Module, define_method));

  // Bootstrap everything
  set_attribute(CLASS(Class), ATOM("name"), PUBLIC, IR_STRING("Class"));
  set_attribute(CLASS(Module), ATOM("name"), PUBLIC, IR_STRING("Module"));
  set_attribute(CLASS(Object), ATOM("name"), PUBLIC, IR_STRING("Object"));
  set_attribute(CLASS(Atom), ATOM("name"), PUBLIC, IR_STRING("Atom"));
  set_attribute(CLASS(Function), ATOM("name"), PUBLIC, IR_STRING("Function"));
  set_attribute(CLASS(String), ATOM("name"), PUBLIC, IR_STRING("String"));

  CLASS(Array) = construct(CLASS(Class));
  CLASS(NilClass) = construct(CLASS(Class));
  CLASS(Integer) = construct(CLASS(Class));
  
  set_attribute(CLASS(Array), ATOM("superclass"), PUBLIC, CLASS(Object));
  // NilClass Inherits from itself -- that way stuff defined on object doesn't affect it.
  set_attribute(CLASS(NilClass), ATOM("superclass"), PUBLIC, CLASS(NilClass));
  set_instance_attribute(CLASS(NilClass), ATOM("__get__"), PUBLIC, get);
  set_attribute(CLASS(Integer), ATOM("superclass"), PUBLIC, CLASS(Object));
  set_attribute(CLASS(Array), ATOM("name"), PUBLIC, IR_STRING("Array"));
  set_attribute(CLASS(NilClass), ATOM("name"), PUBLIC, IR_STRING("NilClass"));
  set_attribute(CLASS(Integer), ATOM("name"), PUBLIC, IR_STRING("Integer"));

  // Init Integer
  other = argument_new(ATOM("other"), NULL, 0);
  fix_plus = FUNCTION(ATOM("__add__"), list_new(other), dict_new(ObjectHashsize), iridium_method_name(Integer, __add__));
  set_instance_attribute(CLASS(Integer), ATOM("__add__"), PUBLIC, fix_plus);
  fix_inspect = FUNCTION(ATOM("inspect"), NULL, dict_new(ObjectHashsize), iridium_method_name(Integer, inspect));
  set_instance_attribute(CLASS(Integer), ATOM("inspect"), PUBLIC, fix_inspect);
  DEF_METHOD(CLASS(Integer), "__sub__", ARGLIST(argument_new(ATOM("other"), NULL, 0)), iridium_method_name(Integer, __sub__));
  DEF_METHOD(CLASS(Integer), "__eq__", ARGLIST(argument_new(ATOM("other"), NULL, 0)), iridium_method_name(Integer, __eq__));
  DEF_METHOD(CLASS(Integer), "__neq__", ARGLIST(argument_new(ATOM("other"), NULL, 0)), iridium_method_name(Integer, __neq__));
  DEF_METHOD(CLASS(Integer), "__lt__", ARGLIST(argument_new(ATOM("other"), NULL, 0)), iridium_method_name(Integer, __lt__));
  DEF_METHOD(CLASS(Integer), "__gt__", ARGLIST(argument_new(ATOM("other"), NULL, 0)), iridium_method_name(Integer, __gt__));
  DEF_METHOD(CLASS(Integer), "__leq__", ARGLIST(argument_new(ATOM("other"), NULL, 0)), iridium_method_name(Integer, __leq__));
  DEF_METHOD(CLASS(Integer), "__geq__", ARGLIST(argument_new(ATOM("other"), NULL, 0)), iridium_method_name(Integer, __geq__));
  DEF_METHOD(CLASS(Integer), "hash", ARGLIST(), iridium_method_name(Integer, hash));

  // Init nil
  nil_inspect = FUNCTION(ATOM("inspect"), NULL, dict_new(ObjectHashsize), iridium_method_name(NilClass, inspect));
  set_instance_attribute(CLASS(NilClass), ATOM("inspect"), PUBLIC, nil_inspect);
  DEF_METHOD(CLASS(NilClass), "to_s", ARGLIST(), iridium_method_name(NilClass, inspect));
  DEF_METHOD(CLASS(NilClass), "__eq__", ARGLIST(argument_new(ATOM("other"), NULL, 0)), iridium_method_name(Object, __eq__));
  DEF_METHOD(CLASS(NilClass), "__neq__", ARGLIST(argument_new(ATOM("other"), NULL, 0)), iridium_method_name(Object, __neq__));

  // Init Atom
  atom_inspect = FUNCTION(ATOM("inspect"), NULL, dict_new(ObjectHashsize), iridium_method_name(Atom, inspect));
  set_instance_attribute(CLASS(Atom), ATOM("inspect"), PUBLIC, atom_inspect);
  DEF_METHOD(CLASS(Atom), "to_s", ARGLIST(), iridium_method_name(Atom, to_s));

  // Init Function
  func_inspect = FUNCTION(ATOM("inspect"), NULL, dict_new(ObjectHashsize), iridium_method_name(Function, inspect));
  set_instance_attribute(CLASS(Function), ATOM("inspect"), PUBLIC, func_inspect);

  // Init String
  str_inspect = FUNCTION(ATOM("inspect"), ARGLIST(), dict_new(ObjectHashsize), iridium_method_name(String, inspect));
  set_instance_attribute(CLASS(String), ATOM("inspect"), PUBLIC, str_inspect);
  DEF_METHOD(CLASS(String), "to_s", ARGLIST(), iridium_method_name(String, to_s));

  // Init Array
  DEF_METHOD(CLASS(Array), "inspect", ARGLIST(), iridium_method_name(Array, inspect));
  DEF_METHOD(CLASS(Array), "reduce", ARGLIST(argument_new(ATOM("accumulator"), NULL, 0), argument_new(ATOM("fn"), NULL, 0)), iridium_method_name(Array, reduce));
  DEF_METHOD(CLASS(Array), "__get_index__", ARGLIST(argument_new(ATOM("index"), NULL, 0)), iridium_method_name(Array, __get_index__));
  DEF_METHOD(CLASS(Array), "__set_index__", ARGLIST(argument_new(ATOM("index"), NULL, 0), argument_new(ATOM("value"), NULL, 0)), iridium_method_name(Array, __set_index__));
  DEF_FUNCTION(CLASS(Array), "new", ARGLIST(argument_new(ATOM("args"), NULL, 1)), iridium_classmethod_name(Array, new));
  DEF_METHOD(CLASS(Array), "push", ARGLIST(argument_new(ATOM("value"), NULL, 0)), iridium_method_name(Array, push));
  DEF_METHOD(CLASS(Array), "unshift", ARGLIST(argument_new(ATOM("value"), NULL, 0)), iridium_method_name(Array, unshift));

  // Init Boolean
  CLASS(Boolean) = send(CLASS(Class), "new", IR_STRING("Boolean"));
  ir_cmp_true = send(CLASS(Boolean), "new");
  ir_cmp_false = send(CLASS(Boolean), "new");
  DEF_FUNCTION(ir_cmp_true, "inspect", ARGLIST(), iridium_classmethod_name(true, inspect));
  DEF_FUNCTION(ir_cmp_false, "inspect", ARGLIST(), iridium_classmethod_name(false, inspect));

  // TODO: Add full exception support
  CLASS(Exception) = send(CLASS(Class), "new", IR_STRING("Exception"));
  DEF_METHOD(CLASS(Exception), "initialize", ARGLIST(argument_new(ATOM("message"), NIL, 0)), iridium_method_name(AttributeError, initialize));
  DEF_METHOD(CLASS(Exception), "reason", ARGLIST(), iridium_method_name(AttributeError, reason));

  // Init AttributeError
  CLASS(AttributeError) = send(CLASS(Class), "new", IR_STRING("AttributeError"), CLASS(Exception));

  CLASS(NameError) = send(CLASS(Class), "new", IR_STRING("NameError"), CLASS(Exception));

  CLASS(TypeError) = send(CLASS(Class), "new", IR_STRING("TypeError"), CLASS(Exception));

  CLASS(ArgumentError) = send(CLASS(Class), "new", IR_STRING("ArgumentError"), CLASS(Exception));

  DEF_METHOD(CLASS(Module), "include", ARGLIST(argument_new(ATOM("module"), NULL, 0)), iridium_method_name(Module, include));
  no_instance_attribute(CLASS(Module), ATOM("new"));

  define_constant(ATOM("Class"), CLASS(Class));
  define_constant(ATOM("Object"), CLASS(Object));
  define_constant(ATOM("Module"), CLASS(Module));
  define_constant(ATOM("Array"), CLASS(Array));
  define_constant(ATOM("Integer"), CLASS(Integer));
  define_constant(ATOM("String"), CLASS(String));
  define_constant(ATOM("Atom"), CLASS(Atom));
  define_constant(ATOM("Function"), CLASS(Function));
  define_constant(ATOM("Exception"), CLASS(Exception));
  define_constant(ATOM("AttributeError"), CLASS(AttributeError));
  define_constant(ATOM("NameError"), CLASS(NameError));
  define_constant(ATOM("TypeError"), CLASS(TypeError));
  define_constant(ATOM("Boolean"), CLASS(Boolean));
  define_constant(ATOM("NilClass"), CLASS(NilClass));
  define_constant(ATOM("ArgumentError"), CLASS(ArgumentError));
}

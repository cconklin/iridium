#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
// #include <gc.h>
#include <string.h>
#include <setjmp.h>

#ifndef OBJECT_H
#define OBJECT_H

// HACK around GC not linking
#define GC_MALLOC(n) calloc(1, n)
#define GC_REALLOC(p, n) realloc(p, n)

// Use the array library to get args
#include "lib/array.h"
// Use the list library for module lookup
#include "lib/linked_list.h"
// Use the dict library for attributes
#include "lib/dict.h"
// Use the str_dict library for atoms
#include "lib/str_dict.h"
// Use the stack library for exceptions
#include "lib/stack.h"

// Magic number to identify objects
#define MAGIC 0x0EFFACED

// Enum for attribute access levels
#define PUBLIC    0
#define PRIVATE   1

// Macros for methods and method names
#define iridium_method_name(class, name) Iridium_##class##_##name
#define iridium_classmethod_name(class, name) iridium_method_name(class, name)

#define iridium_method(class, name) object iridium_method_name(class, name)(struct dict * locals)
#define iridium_classmethod(class, name) iridium_method(class, name)

// Locals
#define local(name) dict_get(locals, ATOM(name))

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

// TODO define the CLASS macro
#define CLASS(name) name

// Macro for superclasses
#define superclass(class) get_attribute(class, ATOM("superclass"), PUBLIC)

// Macro for ATOMS
#define ATOM(name) ((! strcmp(name, "self")) ? SELF_ATOM : _ATOM(name))
#define SELF_ATOM (_SELF_ATOM ? _SELF_ATOM : (_SELF_ATOM = create_self_atom()))

// Declarations

object Class;
object Object;
object Function;
object Atom;
object Tuple;
object NilClass;
object Fixnum;

object nil = NULL;

iridium_classmethod(Class, new);
iridium_method(Class, initialize);
iridium_method(Object, __get__);
iridium_method(Object, __set__);
iridium_method(Function, __call__);
iridium_classmethod(Atom, new);

object get_attribute(object, void *, unsigned char);
object _ATOM(char * name);
object create_self_atom();
object create_nil();
object construct(object class);
object FUNCTION(object name, struct list * args, struct dict * bindings, object (* func)(struct dict *));
object TUPLE(struct array * values);
int INT(object);
object FIXNUM(int);

object invoke(object obj, char * name, struct array * args);
object calls(object callable, struct array * args);

char * str(object);
struct array * destructure(struct array *, object);
struct dict * process_args(object, struct array *);

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
  while (1) {
    if (isA(obj, class)) {
      return 1;
    }
    if (class == superclass(class)) {
      break;
    }
    class = superclass(class);
  }
  return 0;
}

// arg
// Converts an object into an argument

struct array *
arg(object a) {
  struct array * ary = array_new();
  array_set(ary, 0, a);
  return ary;
}

// instance_attribute lookup
// Looks for an instance attribute of a Class, looking at included modules and superclasses, NULL on failure
// Helper function of attribute_lookup

// Macro for when no attribute is found
#define no_result ((! result) || (result -> access > access))

// Macro for whether a module list has any modules
#define modules_present(mod_list) ((mod_list) ? 0 : (list_length(mod_list) > 0))

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
      for (module = list_head(modules); list_tail(modules) && no_result; modules = list_tail(modules), module = list_head(modules)) {
        result = instance_attribute_lookup(module, attribute, access);
      }
    }
  }
  // Catches the case where result is not NULL, but has the wrong access level
  if (no_result) return NULL;
  return result;
}

// get_attribute
// Returns the value of an object attribute, nil on failure

// Declaration of helper function
iridium_attribute attribute_lookup(object, void *, unsigned char);

object
get_attribute(object receiver, void * attribute, unsigned char access) {
  iridium_attribute result = attribute_lookup(receiver, attribute, access);
  return (result && result -> real) ? result -> value : NIL ;
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

// internal_get_attribute
// Returns an attribute of obj which is INTERNAL
#define internal_get_attribute(obj, attr, cast) ((cast)(dict_get(((object) obj) -> internal_attributes, attr)))

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
  return value;
}

// internal_set_attribute
// mutates the receiver, setting attribute with INTERNAL access to a new value
// NOTE: could run into issue where attribute is set before with a lower access (which means that an attribute that ought to be internal isn't)
#define internal_set_attribute(receiver, attribute, value) dict_set(receiver -> internal_attributes, attribute, value)

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
  return FUNCTION(name, args, locals, c_func);
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

// Function for generic invocations
// Used by the translator

object calls(object callable, struct array * args) {
  object get_function = get_attribute(callable, ATOM("__get__"), PUBLIC);
  object call_function;
  struct dict * bindings;
  object ( * func )(struct dict *) = internal_get_attribute(get_function, ATOM("function"), object (*)(struct dict *));
  // get_function IS an Iridium Function
  // TODO raise exception if not
  // invoke the get_function to get the call_function
  call_function = func(dict_with(bind_self(callable), ATOM("name"), ATOM("__call__")));
  // call_function IS an Iridium Function
  // TODO raise exception if not
  bindings = process_args(call_function, args);
  func = internal_get_attribute(call_function, ATOM("function"), object (*)(struct dict *));
  assert(func);
  return func(bindings);
}

// Converts iridium strings to C strings
char * str(object string) {
  return internal_get_attribute(string, ATOM("string"), char *);
}

// Destructures tuples into struct array *
struct array * destructure(struct array * args, object argument_tuple) {
  struct array * tuple_args = internal_get_attribute(argument_tuple, ATOM("array"), struct array *);
  return array_merge(args, tuple_args);
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
  int length = 0;
  int arg_length = args -> length;
  unsigned int required = 0;
  struct IridiumArgument * this_arg;
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
      assert(/* There is a required argument after an optional one */ 0);
    }
    iter_arg_list = list_tail(iter_arg_list);
    index ++;
  }
  length = index;
  // ensure that the argument list does not have multiple splatted args
  assert(splat < 2);
  // ensure that the arity is sufficient
  assert(arg_length >= required);
  // if there is no splat, ensure that the arity is not too large
  if (splat == 0) {
    assert(arg_length <= length);
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
      while (index < (arg_length - (length - splat))) {
        array_push(partial_args, array_shift(args));
        index ++;
      }
      // Splatted argument
      dict_set(argument_values, this_arg -> name, TUPLE(partial_args));
    }
    iter_arg_list = list_tail(iter_arg_list);
  } 
  return dict_merge(closed_values, argument_values);
}


// class Class

// Class.new
// Constructor of objects
// Inputs: locals (struct dict *, dictionary of local variables used in closures)
//            Locals used: self, args
// Output:  obj (object)

// function new(* args)
iridium_classmethod(Class, new) {

  // Value of the receiver
  object self = local("self");
  // Value of the args
  object args = local("args"); // tuple
  
  // Container for the object under construction
  object obj = construct(self);

  // Send off to the object initialize method
  invoke(obj, "initialize", destructure(array_new(), args));
  
  // Now that the object is constructed and initialized, return it
  return obj;
  
}

// Class#initialize
// Initializes new classes with a superclass
// Inputs: locals (struct dict *, dictionary of local variables used in closures)
//            Locals used: self
//          args
//            Arity: 1/0
//            superclass: Object
// Output:  obj (object)

iridium_method(Class, initialize) {
  // set self
  object self = local("self");
  object superclass = local("superclass");
  // set super to the passed value, if present
  set_attribute(self, ATOM("superclass"), PUBLIC, superclass);
  return NIL;
}

// class Object

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
  
  // If the attribute is found, is it a function?
  if (isA(attribute, CLASS(Function))) {
    vars = internal_get_attribute(attribute, ATOM("bindings"), struct dict *);
    // The function better have some bindings, even if it is an empty dictionary.
    assert(vars);
    attribute = function_bind(attribute, dict_with(vars, ATOM("self"), self));
  }

  return attribute;
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
  object args = local("args"); // tuple
  
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
  internal_set_attribute(self, ATOM("bindings"), bindings);
  internal_set_attribute(self, ATOM("function"), func);
  return self;
}

// class Atom

// Atom.new
// Creates an atom
// Arity: 1
//  name (Iridium String)
void * ATOM_TABLE_KEY = NULL;
#define ATOM_HASHSIZE 100

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

// Create the `:self` atom
object create_self_atom() {
  // Allocate memory for the object
  object self_atom = construct(CLASS(Atom));

  // Add it to the Atom table
  add_atom("self", self_atom);

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
  }
  return atom;
}

// class Tuple

object TUPLE(struct array * values) {
  object tuple = construct(CLASS(Tuple));
  internal_set_attribute(tuple, ATOM("array"), values);
  return tuple;
}

iridium_classmethod(Tuple, new) {
  object args = local("args");
  object tuple = TUPLE(destructure(array_new(), args));
  // Call initialize (which should do nothing unless overidden)
  invoke(tuple, "initialize", destructure(array_new(), args));
  return tuple;
}

// method reduce(accumulator, fn)
iridium_method(Tuple, reduce) {
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

iridium_method(Tuple, __get_index__) {
  object self = local("self");
  object index = local("index"); // Iridium Fixnum
  struct array * ary = internal_get_attribute(self, ATOM("array"), struct array *);
  return array_get(ary, INT(index));
}

iridium_method(Tuple, __set_index__) {
  object self = local("self");
  object index = local("index"); // Iridium Fixnum
  object value = local("value");
  struct array * ary = internal_get_attribute(self, ATOM("array"), struct array *);
  array_set(ary, INT(index), value);
  return value;
}


// class Fixnum

iridium_classmethod(Fixnum, new) {
  object fixnum = local("fixnum");
  return FIXNUM(INT(fixnum));
}

iridium_method(Fixnum, __plus__) {
  object self = local("self");
  object other = local("other");
  return FIXNUM(INT(self) + INT(other));
}

object FIXNUM(int val) {
  object fixnum = construct(Fixnum);
  internal_set_attribute(fixnum, ATOM("value"), val);
  return fixnum;
}

int INT(object fixnum) {
  return internal_get_attribute(fixnum, ATOM("value"), int);
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

// class Exception

struct Exception {
  // Exception Class
  object class;
  // Value that setjmp will return when it is jumped to
  int jump_location;
};

typedef struct ExceptionFrame {
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
      END_ENSURE;
  }
  _handler_count --;
*/

#define ENSURE_JUMP -1
#define END_ENSURE if (_rescuing) handleException(_raised); break
// End the handler first so that if an exception is raised in the `ensure`,
// this handler will not rescue it
#define END_BEGIN(frame) run_else(frame); endHandler(frame); ensure(frame); break
#define END_RESCUE(frame) ensure(frame); break
#define END_ELSE(frame) endHandler(frame); ensure(frame); break
#define ELSE_JUMP -2

// Location that an ensure jumps to if it completes normally when an exception is raised
jmp_buf _handler_env;

#define ensure(e) \
  if ( e -> ensure ) \
    if (! setjmp(_handler_env)) \
      longjmp(e -> env, ENSURE_JUMP);

#define run_else(e) \
  if ( e -> else_block ) \
    if (! setjmp(_handler_env)) \
      longjmp(e -> env, ELSE_JUMP);


// FIXME Not thread-safe
// Global variable to hold exceptions
// Will ALWAYS have a top level handler which will terminate the Iridium program
struct stack * _exception_frames;
// Global variable to hold the exception being raised
object _raised;
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
    frame = (exception_frame) stack_pop(frames);
    if ((e = catchesException(frame, exception))) {
      // Jump to the appropriate handler
      // Binding of the exception to the variable happens during the exception handler
      // Before we jump, we need to indicate that we are no longer handling the exception
      // since a handler has been located
      _rescuing = 0;
      longjmp(frame -> env, e -> jump_location);
    }
    // Run the ensure
    ensure(frame);
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

void return_in_begin_block() {
  struct stack * frames = _exception_frames;
  exception_frame frame;
  assert(frames);
  // Should have at least one handler
  while(! stack_empty(frames)) {
    frame = (exception_frame) stack_pop(frames);
    // Run the ensure
    ensure(frame);
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
}

#endif

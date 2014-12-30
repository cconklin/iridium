#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
// #include <gc.h>
#include <string.h>

// HACK around GC not linking
#define GC_MALLOC(n) calloc(1, n)
#define GC_REALLOC(p, n) realloc(p, n)

// Use the array library to get args
#include "array.h"
// Use the list library for module lookup
#include "linked_list.h"
// Use the str_dict library for atoms
// The dict library comes with it
#include "str_dict.h"

// Magic number to identify objects
#define MAGIC 0x0EFFACED

// Enum for attribute access levels
#define PUBLIC    0
#define PRIVATE   1
#define INTERNAL  2

// Hashsize for object attribute hashes
#define ObjectHashsize 30

// Macro to identify objects
#define isObject(obj) (obj && ((struct IridiumObject *) obj) -> magic == MAGIC)

// Macro to identify types
#define isA(obj, klass) (isObject(obj) && ((struct IridiumObject *) obj) -> class == klass)

// Macro for method calls
// Translates to:
//    obj.name(args)
// Iridium_Object_get() will retun a bound function to the `obj` #"name" method (obj will automatically be passed)
// Invoke function with handle `name` by getting the function from the object `obj`, and bind it to `self` before passing it to the C Function for Iriridum Function calls.
#define invoke(obj, name, args) \
  Iridium_Function_call( \
    dict_with(dict_new(ObjectHashsize), ATOM("self"), Iridium_Object_get( \
      dict_with(dict_new(ObjectHashsize), ATOM("self"), obj), \
      arg(ATOM(name)) \
    ) \
    ), \
  args)

// TODO define NIL macro
#define NIL NULL

// TODO define the CLASS macro
#define CLASS(name) name

// Macro for superclasses
#define superclass(class) get_attribute(class, ATOM("superclass"), PUBLIC)

// Macro for ATOMS
#define ATOM(name) ((! strcmp(name, "self")) ? SELF_ATOM : _ATOM(name))
#define SELF_ATOM (_SELF_ATOM ? _SELF_ATOM : (_SELF_ATOM = create_self_atom()))

// Struct underlying all Iridium objects
typedef struct IridiumObject {
  // Calling card of all objects
  unsigned long long int magic;
  // Pointer to the class description (is also an Iridium Object)
  struct IridiumObject * class;
  // Dictionary of attributes
  struct dict * attributes;
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
  void * value;
} * iridium_attribute;

// Declarations

object Class;
object Function;
object Atom;

object Iridium_Class_new(struct dict *, struct array *);
object Iridium_Object_get(struct dict *, struct array *);
object Iridium_Function_call(struct dict *, struct array *);
object Iridium_Atom_new(struct dict *, struct array *);
object _ATOM(char * name);
object create_self_atom();

// :self atom
object _SELF_ATOM = NULL;

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
    if (modules) {
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
  return result ? result -> value : NIL ;
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
    if (modules) {
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
    if (modules) {
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
#define internal_get_attribute(obj, attr, cast) (cast)(attribute_lookup(obj, attr, INTERNAL) ? (attribute_lookup(obj, attr, INTERNAL) -> value) : NULL)

// set_attribute
// mutates the receiver, setting the attribute with `access` to a new value
object set_attribute(object receiver, void * attribute, unsigned char access, object value) {
  iridium_attribute attr;

  attr = (iridium_attribute) GC_MALLOC(sizeof(struct IridiumAttribute));
  assert(attr);
  attr -> access = access;
  // set the new value
  attr -> value = (void *) value;
  dict_set(receiver -> attributes, attribute, attr);
  return value;
}

// internal_set_attribute
// mutates the receiver, setting attribute with INTERNAL access to a new value
// NOTE: could run into issue where attribute is set before with a lower access (which means that an attribute that ought to be internal isn't)
#define internal_set_attribute(receiver, attribute, value) (__typeof__(value)) set_attribute(receiver, attribute, INTERNAL, (object) value) 

// function_bind
// Bind locals to functions
// Does not mutate the receiver
// accepts: func (object, Iridium Function), locals (struct dict *)
// returns: object (Iridirum Function)
object
function_bind(object func, struct dict * locals) {
  // Function with bound locals
  object bound_function;

  // Pointer to C function
  // TODO determine type
  void * c_func;

  // Create a new function object
  bound_function = (object) GC_MALLOC(sizeof(struct IridiumObject));
  // Ensure that it was created
  assert(bound_function);
  // Set the class of the object to Function
  bound_function -> class = CLASS(Function);

  // Set the locals
  internal_set_attribute(bound_function, ATOM("bindings"), locals);

  // Set the function
  c_func = internal_get_attribute(func, ATOM("function"), __typeof__(c_func));
  assert(c_func);
  internal_set_attribute(bound_function, ATOM("function"), c_func);

  // Return the new bound function
  return bound_function;
}


// class Class

// Class#new
// Constructor of objects
// Inputs:  self (object, will be an instance of Class)
//          locals (struct dict *, dictionary of local variables used in closures)
//            (Note: since this isn't a closure, locals will not be used)
//          args (array of objects)
// Output:  obj (object)

object
Iridium_Class_new(struct dict * locals, struct array * args) {

  // Value of the receiver
  object self = dict_get(locals, ATOM("self"));
  
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
  obj -> class = self;
  
  // Initialize the attribute dictionary
  obj -> attributes = dict_new(ObjectHashsize);
  
  // Send off to the object initialize method
  invoke(obj, "initialize", args);
  
  // Now that the object is constructed and initialized, return it
  return obj;
  
}

// class Object

// Object#__get__
// Gets attributes of objects, looking up the object hierarchy
// Inputs:  self (object, any arbitrary Iridium object)
//          locals (struct dict *, dictionary of local variables used in closures)
//            Locals used: self
//          args (array of objects)
// Output:  obj (object)
// Iridium Example: obj.get(:value) # => :some_value


object
Iridium_Object_get(struct dict * locals, struct array * args) {

  // Value of the receiver
  object self = dict_get(locals, ATOM("self"));
  
  // Dictionary of attribute locals, if attribute is a function
  struct dict * vars;

  // Grab the key from the argument array
  void * key = array_get(args, 0);
  
  // Look for the attribute
  object attribute = get_attribute(self, key, PUBLIC);
  
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
//          args (array of objects)
// Output:  obj (object, the value of the passed attribute)
// Iridium Example: obj.set(:value, :some_value) # => :some_value

object
Iridium_Object_set(struct dict * locals, struct array * args) {

  // Value of the receiver
  object self = dict_get(locals, ATOM("self"));

  // Grab the key from the argument array
  void * key = array_get(args, 0);

  // Get the attribute from the argument array
  object attribute = (object) array_get(args, 1);

  // Set the attribute on the object as a public attribute
  set_attribute(self, key, PUBLIC, attribute);

  return attribute;
}

// class Function

// Function#__call__
// Invokes a function

object
Iridium_Function_call(struct dict * locals, struct array * args) {
  // Value of the receiver
  object self = dict_get(locals, ATOM("self"));

  // Get the corresponding C function
  object ( * func )(struct dict *, struct array *) = internal_get_attribute(self, ATOM("function"), object (*)(struct dict *, struct array *));
  assert(func);

  // Set the locals to that of the function (retrieve closed values)
  locals = internal_get_attribute(self, ATOM("bindings"), struct dict *);
  assert(locals);

  // Check the length of `args` agains the arity of the function object
  assert((args -> length + 1) == internal_get_attribute(self, ATOM("arity"), int));

  // Invoke the function
  return func(dict_copy(locals), args);
}

// class Atom

// Atom.new
// Creates an atom
// TODO fix what will become an infinite loop of atoms being constructed in the atom loop
// TODO ensure that the atom table is created when the class is

void * ATOM_TABLE_KEY = NULL;
#define ATOM_HASHSIZE 100

object
Iridium_Atom_new(struct dict * locals, struct array * args) {
  // Get the name of the atom
  char * name = array_get(args, 0);

  // Create the atom
  return ATOM(name);
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
    return (struct dict *) (((iridium_attribute) dict_get(atom_class -> attributes, ATOM_TABLE_KEY)) -> value);
  }

}

void add_atom(char * name, object atom) {
  str_dict_set(ATOM_TABLE(), name, atom);
}

// Create the `:self` atom
object create_self_atom() {
  // Allocate memory for the object
  object self_atom = (object) GC_MALLOC(sizeof(struct IridiumObject));

  // Ensure that memory was allocated
  assert(self_atom);

  // Set the object's magic number
  self_atom -> magic = MAGIC;

  // Set the object's class to Atom
  self_atom -> class = CLASS(Atom);

  // Initialize the attribute dictionary
  self_atom -> attributes = dict_new(ObjectHashsize);

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
    // This is an optimized call to super, effectively.
    // Allocate memory for the object
    atom = (object) GC_MALLOC(sizeof(struct IridiumObject));

    // Ensure that memory was allocated
    assert(atom);

    // Set the object's magic number
    atom -> magic = MAGIC;

    // Set the object's class to Atom
    atom -> class = CLASS(Atom);

    // Initialize the attribute dictionary
    atom -> attributes = dict_new(ObjectHashsize);

    // Add it to the Atom table
    add_atom(name, atom);
  }
  return atom;
}
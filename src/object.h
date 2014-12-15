#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <gc.h>

// Use the dict library to access attributes
#include "dict.h"
// Use the array library to get args
#include "array.h"
// Use the list library for module lookup
#include "linked_list.h"

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
#define invoke(obj, name, locals, args) Iridium_Function_call(Iridium_Object_get(obj, locals, arg(ATOM(name))), locals, args)

// TODO define NIL macro
#define NIL NULL

// TODO define the ATOM macro
#define ATOM(name) NULL

// TODO define the CLASS macro
#define CLASS(name) name

// Macro for superclasses
#define superclass(class) get_attribute(class, ATOM("superclass"), PUBLIC)

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
  // list of extended modules
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

object Iridium_Class_new(object, struct dict *, struct array *);
object Iridium_Object_get(object, struct dict *, struct array *);
object Iridium_Function_call(object, struct dict *, struct array *);


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
  }
  // Catches the case where result is not NULL, but has the wrong access level
  if (no_result) return NULL;
  return result;
}

// internal_attribute_lookup
// Returns an attribute of obj which is INTERNAL
#define internal_attribute_lookup(obj, attr, cast) (attribute_lookup(obj, attr, INTERNAL) ? (cast)(attribute_lookup(obj, attr, INTERNAL) -> value) : NULL)

// set_attribute
// mutates the receiver, setting the attribute with `access` to a new value
object set_attribute(object receiver, void * attribute, unsigned char access, object value) {
  iridium_attribute attr = attribute_lookup(receiver, attribute, access);
  // TODO replace with exception
  assert(attr);
  // set the new value
  attr -> value = (void *) value;
  return value;
}

// function_bind
// Bind locals to functions
// accepts: func (object, Iridium Function), locals (struct dict *)
// returns: object (Iridirum Function)
object
function_bind(object, struct dict *);


// class Class

// Class#new
// Constructor of objects
// Inputs:  self (object, will be an instance of Class)
//          locals (struct dict *, dictionary of local variables used in closures)
//            (Note: since this isn't a closure, locals will not be used)
//          args (array of objects)
// Output:  obj (object)

object
Iridium_Class_new(object self, struct dict * locals, struct array * args) {
  
  // Container for the object under construction
  object obj;
  
  // Allocate memory for the object
  obj = (object) GC_MALLOC(sizeof(struct IridiumObject));
  
  // Ensure that memory was allocated
  assert(obj);
  
  // Set the objects class to self (since this is a method on an instance of
  // Class, which implies that self is a class)
  obj -> class = self;
  
  // Initialize the attribute dictionary
  obj -> attributes = dict_new(ObjectHashsize);
  
  // Send off to the object initialize method
  invoke(obj, "initialize", locals, args);
  
  // Now that the object is constructed and initialized, return it
  return obj;
  
}

// class Object

// Object#__get__
// Gets attributes of objects, looking up the object hierarchy
// Inputs:  self (object, any arbitrary Iridium object)
//          locals (struct dict *, dictionary of local variables used in closures)
//            (Note: since this isn't a closure, locals will not be used)
//          args (array of objects)
// Output:  obj (object)


object
Iridium_Object_get(object self, struct dict * locals, struct array * args) {
  
  // Dictionary for self to be bound to the attribute if it is a function
  struct dict * binding;
  // Dictionary of attribute locals, if attribute is a function
  struct dict * vars;

  // Grab the key from the argument array
  void * key = array_get(args, 0);
  
  // Look for the attribute
  object attribute = get_attribute(self, key, PUBLIC);
  
  // If the attribute is found, is it a function?
  if (isA(attribute, CLASS(Function))) {
    binding = dict_new(ObjectHashsize);
    dict_set(binding, ATOM("self"), self);
    vars = internal_attribute_lookup(attribute, ATOM("bindings"), struct dict *);
    // The function better have some bindings, even if it is an empty dictionary.
    assert(vars);
    attribute = function_bind(attribute, dict_merge(vars, binding));
  }

  return attribute;
}
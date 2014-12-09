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
#define superclass(class) attribute_lookup(class, ATOM("superclass"))

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

object
instance_attribute_lookup(object receiver, void * attribute) {
  // Try and get it from the objects instance attributes
  object result = dict_get(receiver -> instance_attributes, attribute);
  struct list * modules;
  object module;
  
  if (! result) {
    // Attribute was not found
    // Now look at any modules the object included
    modules = receiver -> included_modules;
    if (modules) {
      // If there are any included modules
      // Search all the modules until the attribute is found
      for (module = list_head(modules); list_tail(modules) && result == NULL; modules = list_tail(modules), module = list_head(modules)) {
        result = instance_attribute_lookup(module, attribute);
      }
    }
  }
  
  return result;
}

// attribute_lookup
// Returns the value of an object attribute, nil on failure

object
attribute_lookup(object receiver, void * attribute) {
  // Look for attribute among the objects attributes
  object result = dict_get(receiver -> attributes, attribute);
  struct list * modules;
  object module;
  object class;
  
  
  if (! result) {
    // Attribute was not found
    // Now look at any modules the object extended
    modules = receiver -> extended_modules;
    if (modules) {
      // If there are any extended modules
      // Search all the modules until the attribute is found
      for (module = list_head(modules); list_tail(modules) && result == NULL; modules = list_tail(modules), module = list_head(modules)) {
        result = instance_attribute_lookup(module, attribute);
      }      
    }
    if (! result) {
      // Not in the extended modules instance attributes
      // Check the included modules attributes
      modules = receiver -> included_modules;
      if (modules) {
        // If there are any included modules
        // Search all the modules until the attribute is found
        for (module = list_head(modules); list_tail(modules) && result == NULL; modules = list_tail(modules), module = list_head(modules)) {
          result = attribute_lookup(module, attribute);
        }      
      }
    }
    // If it is not in a module and the receiver is a class, look up the superclass chain
    if (! result && isA(receiver, CLASS(Class))) {
      // Store the receiver in a temp so that it is not modified
      class = receiver;
      // Look up each superclass
      while (! result && class != superclass(class)) {
        class = superclass(class);
        // Look in the superclass for the attribute
        result = attribute_lookup(class, attribute);
      }      
    }
    if (! result) {
      // None of the  modules had the attribute, check the class chain
      class = receiver -> class;
      // instance_attribute_lookup also checks for the classes included modules
      result = instance_attribute_lookup(class, attribute);
      // Check the superclass
      // Note: superclass is a normal attribute -- it undergoes attribute lookup
      // Ensure that all classes have a superclass, or this will result in an infinite loop
      // Keep searching until the attribute is found, or the class chain is at the end
      while (! result && class != superclass(class)) {
        class = superclass(class);
        result = instance_attribute_lookup(class, attribute);
      }
    }
  }
  return result ? result : NIL ;
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
  
  // Grab the key from the argument array
  void * key = array_get(args, 0);
  
  // Look for the attribute
  object attribute = attribute_lookup(self, key);
  
  // If the attribute is found, is it a function?
  if (isA(attribute, CLASS(Function))) {
    // TODO Bind self to function
  }

  return attribute;
}
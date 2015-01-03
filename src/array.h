/*
  array.h
  Implements the array structure used in function argument lists and tuples
*/

#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h>
// #include <gc.h>
#include <assert.h>

// HACK around GC not linking
#ifndef GC
#define GC
#define GC_MALLOC(n) calloc(1, n)
#define GC_REALLOC(p, n) realloc(p, n)
#endif

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

struct array {
  // Pointer to the array
  void ** elements;
  // Size of the array (in # of elements)
  unsigned int size;
  // index of the last element in the array
  unsigned int length;
};

// array_new
// Create a new, empty array
struct array * array_new(void) {
  
  // Allocate space for the array structure
  struct array * ary = (struct array *) GC_MALLOC(sizeof(struct array));
  
  // Ensure that memory was allocated
  assert(ary);
  
  // Set the size to 10
  ary -> size = 10;
  
  // Set the array length to 0 (since there are no elements)
  ary -> length = 0;
  
  // Allocate space for the elements
  ary -> elements = GC_MALLOC(10 * sizeof(void *));
  
  // Ensure that memory was allocated
  assert(ary -> elements);
  
  return ary;
}

// array_resize
// Ensures that the internal array is of the correct size for adding an element

void array_resize(struct array * ary, unsigned int size) {
  // Check if space for index has been allocated
  if ( size >= ary -> size ) {
    // Allocate to 1.5 times the needed size
    // This sacrifices space in exchange for needing fewer reallocations
    ary -> size = size + size / 2;
    
    // Reallocate the elements memory
    ary -> elements = GC_REALLOC(ary -> elements, ary -> size * sizeof(void *));
    
    // Ensure that memory was allocated
    assert(ary -> elements);  
  }
}

// array_set
// Inputs: ary, index, value
// returns: ary
// Mutate ary so that `value` is at index `index`
struct array * array_set(struct array * ary, unsigned int index, void * value) {
  
  // Ensure that the array has enought memory allocated
  array_resize(ary, index);
  
  // Set the spot in memory
  ary -> elements [index] = value;
  
  // Update the max index
  ary -> length = max(ary -> length, index + 1);
  
  return ary;
  
}

// array_get
// Gets the value at `index`
void * array_get(struct array * ary, unsigned int index) {
  // Ensure that the array is large enough
  if (index >= ary -> size)
    return NULL;
  
  // Return the value at `index`
  return ary -> elements [index];
}

#endif
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
  // index of the first element in the array
  unsigned int start;
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
  
  // Set the start index to 0 (since it hasn't been shifted)
  ary -> start = 0;
  
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
  array_resize(ary, ary -> start + index);
  
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
  if ( ary -> start + index >= ary -> size)
    return NULL;
  
  // Return the value at `index`
  return ary -> elements [ary -> start + index];
}

// array_push
// Push a new value to the end of array
struct array * array_push(struct array * ary, void * value) {
  return array_set(ary, ary -> length, value);
}

// array_shift
// returns the first value of the array, moving the remaining elements one back
void * array_shift(struct array * ary) {
  void * result;
  // Ensure that the array is large enough
  if (ary -> start >= ary -> size)
    return NULL;
  result = array_get(ary, 0);
  ary -> start ++;
  ary -> length --;
  return result;
}

struct array * array_copy(struct array * ary) {
  struct array * new_array = array_new();
  int index;
  for (index = 0; index < ary -> length; index ++) {
    array_set(new_array, index, array_get(ary, index));
  }
  return new_array;
}

struct array * array_merge(struct array * array_1, struct array * array_2) {
  struct array * result = array_copy(array_1);
  int index;
  for (index = 0; index < array_2 -> length; index ++) {
    array_push(result, array_get(array_2 , index));
  }
  return result;
}

#endif
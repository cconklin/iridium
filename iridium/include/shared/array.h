/*
  array.h
  Implements the array structure used in function argument lists and tuples
*/

#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h>
#include <gc.h>
#include <assert.h>

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
struct array * array_new(void);

// array_resize
// Ensures that the internal array is of the correct size for adding an element

void array_resize(struct array * ary, unsigned int size);
// array_set
// Inputs: ary, index, value
// returns: ary
// Mutate ary so that `value` is at index `index`
struct array * array_set(struct array * ary, unsigned int index, void * value);

// array_get
// Gets the value at `index`
void * array_get(struct array * ary, unsigned int index);

// array_push
// Push a new value to the end of array
struct array * array_push(struct array * ary, void * value);

// array_pop
// Returns the element at the end of the array, removing it
void * array_pop(struct array * ary);

// array_shift
// returns the first value of the array, moving the remaining elements one back
void * array_shift(struct array * ary);

struct array * array_copy(struct array * ary);

struct array * array_unshift(struct array * ary, void * value);

struct array * array_merge(struct array * array_1, struct array * array_2);

#endif

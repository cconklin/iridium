/*
  stack.h
  Interface to a stack implementation.
*/

#include "shared/stack.h"

// stack_new
// returns: struct stack *
// Creates a new, empty stack
struct stack * stack_new(void) {
  // Allocate memory for the stack
  struct stack * stack = (struct stack *) malloc(sizeof(struct stack));
  // Ensure that memory was allocated
  assert(stack);
  // Initialize the length to 10
  stack -> length = 10;
  // Initialize the depth (how many items are in the stack) to -1
  // This way, the depth can be incremented before adding the element,
  // and there is an easy check for an empty stack
  stack -> depth = -1;
  // Allocate the stack array
  stack -> stack = (void **) malloc(stack -> length * sizeof(void *));
  // Ensure that the array memory was allocated
  assert(stack -> stack);
  
  return stack;
}

// stack_push
// inputs: stack (struct stack *), data (void *)
// Push an element `data` to the stack `stack`
void stack_push(struct stack * stack, void * data) {
  // Ensure that the depth & length has not overflowed
  assert(stack -> depth >= -1 && stack -> length > 0);
  // Check if there is room in the stack
  if (stack -> depth == stack -> length - 1) {
    // Stack is full
    // Add 10 elements to the stack
    stack -> length += 10;
    // Allocate the memory for the larger stack
    stack -> stack = (void **) realloc(stack -> stack, stack -> length);
    // Ensure that the memory was allocated
    assert(stack -> stack);
  }
  // Increase the depth
  stack -> depth ++ ;
  // Add the data to the stack
  stack -> stack[stack -> depth] = data;
}

// stack_pop
// inputs: stack (struct stack *)
// returns: void *
// Pop an element off the top of the stack, removing it from the stack
void * stack_pop(struct stack * stack) {
  // Element that is removed from the stack
  void * data;
  // Set `data` to the top of the stack
  data = stack_top(stack);
  // Decrement the depth of the stack
  stack -> depth --;
  
  return data;
}

// stack_top
// inputs: stack (struct stack *)
// returns: void *
// Return the top element of the stack, without modifying the stack
void * stack_top(struct stack * stack) {
  // Ensure that the stack has data
  assert(! stack_empty(stack));
  // Get and return the top element
  return stack -> stack [stack -> depth];
}

// stack_destroy
// inputs: stack (struct stack *)
// Frees the memory used by a stack
// Note: does not free the data of the elements in the stack
void stack_destroy(struct stack * stack) {
  // Free the internal stack structure
  free(stack -> stack);
  // Free the stack
  free(stack);
}

int stack_empty(struct stack * s) {
  return (s -> depth == -1);
} 

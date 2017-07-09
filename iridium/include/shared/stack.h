/*
  stack.h
  Interface to a stack implementation.
*/

#include <stdlib.h>
#include <assert.h>
#pragma once

#include <gc.h>

struct stack {
  int length;
  int depth;
  void ** stack;
};

// stack_empty
// inputs: stack (struct stack *)
// returns: int (boolean)
// Determines if there are no elements in the stack
int stack_empty(struct stack *);
struct stack * stack_new(void);
void stack_push(struct stack *, void * data);
void * stack_pop(struct stack *);
void * stack_top(struct stack *);
void stack_destroy(struct stack *);

/*
  str_dict.h
  Implementation of dictionaries with string keys (for use with Atoms)
  Uses a hash to store key-value pairs
    Keys: char * (C strings)
    Values: void * (pointer to any Iridium Object)
*/

#ifndef STR_DICT_H
#define STR_DICT_H

#include <stdlib.h>
#include <assert.h>
// #include <gc.h>
#include <string.h>
#include "dict.h"

// HACK around GC not linking
#ifndef GC
#define GC
#define GC_MALLOC(n) calloc(1, n)
#define GC_REALLOC(p, n) realloc(p, n)
#endif

struct dict * str_dict_new(unsigned int hashsize);
struct dict_entry * str_lookup(struct dict * h, char * key);
void str_dict_set(struct dict * h, char * key, void * value);
void * str_dict_get(struct dict * h, char * key);
void str_dict_delete(struct dict * h, char * key);
void str_dict_destroy(struct dict * h);
struct dict * str_dict_merge(struct dict *, struct dict *);
struct dict * str_dict_with(struct dict *, char *, void *);
struct dict * str_dict_copy(struct dict *);

unsigned int str_hash(struct dict * h, char * key);

#endif
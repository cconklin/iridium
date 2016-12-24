/*
  dict.h
  Basic library underlying the Dictionary class and the attribute system of all objects.
  Uses a hash to store key-value pairs
    Keys: void * (addresses of memory, expected to be Iridium Atoms)
    Values: void * (pointer to any Iridium Object)
*/

#include <stdlib.h>
#include <assert.h>
#include <math.h>
// #include <gc.h>

#ifndef DICT_H
#define DICT_H

// HACK around GC not linking
#ifndef GC
#define GC
#define GC_MALLOC(n) calloc(1, n)
#define GC_REALLOC(p, n) realloc(p, n)
#endif


#define hash(h, key) ((unsigned long long int) key) % h -> hashsize

union dict_value {
  void * ptr;
  double flt;
  long long int integral;
};

struct dict_entry {
  struct dict_entry * next;
  void * key; // Use an address of memory as the key
  union dict_value value; // Store some arbitrary data
};

struct dict {
  unsigned int hashsize;
  struct dict_entry ** hashtab;
};

struct dict * dict_new(unsigned int hashsize);
struct dict_entry * lookup(struct dict * h, void * key);
void dict_set(struct dict * h, void * key, void * value);
void * dict_get(struct dict * h, void * key);
void dict_delete(struct dict * h, void * key);
void dict_destroy(struct dict * h);
struct dict * dict_merge(struct dict *, struct dict *);
struct dict * dict_with(struct dict *, void *, void *);
struct dict * dict_copy(struct dict *);
void dict_set_integral(struct dict * h, void * key, long long int value);
void dict_set_flt(struct dict * h, void * key, double value);
long long int dict_get_integral(struct dict * h, void * key);
double dict_get_flt(struct dict * h, void * key);

#endif
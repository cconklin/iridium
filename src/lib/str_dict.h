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

unsigned int str_hash(struct dict * h, char * key) {
  unsigned int hashval; 
  unsigned int index;
  unsigned int length = strlen(key);
  for (index = 0; index < length; index ++) {
    // Create an integer based on the name
    // 48 is the ASCII offset of '0' -- choose 47 so that each character is non-zero
    hashval += (key[index] - 47) * (index + 1);
  }
  return hashval % (h -> hashsize);
}

struct dict * str_dict_new(unsigned int hashsize) {
  struct dict * h;
  h = GC_MALLOC(sizeof(struct dict));
  assert(h);
  h -> hashsize = hashsize;
  h -> hashtab = (struct dict_entry **) GC_MALLOC(hashsize * sizeof(struct dict_entry *));
  assert(h -> hashtab);
  return h;
}

void str_dict_set(struct dict * h, char * key, void * value) {
  struct dict_entry * entry ;
  unsigned int hashval;
  hashval = str_hash(h, key);
  // associate key with value in hash
  if (!(entry = str_lookup(h, key))) {
    // not in the hash
    entry = (struct dict_entry *) GC_MALLOC(sizeof(struct dict_entry));
    assert(entry); // Ensure that entry has been allocated.
    entry -> next = (h -> hashtab)[hashval]; // Make the head of the entry linked list
    (h -> hashtab)[hashval] = entry; // Make this the first element seen when looked up in the hashtab array.
  }
  entry -> value = value;
  entry -> key = GC_MALLOC(strlen(key) + 1);
  assert(entry -> key);
  strcpy(entry -> key, key);
}

struct dict * str_dict_with(struct dict * h, char * key, void * value) {
  struct dict * result = str_dict_copy(h);
  str_dict_set(result, key, value);
  return result;
}


struct dict_entry * str_lookup(struct dict * h, char * key) {
  // Find the str_dict_entry element corresponding to a key
  // Return NULL if key not present
  struct dict_entry * entry ;
  for (entry = (h -> hashtab[str_hash(h, key)]); entry != NULL; entry = entry -> next)
    if (! strcmp(key, entry -> key))
      return entry; // Found the entry with key: key
  return NULL; // No entries found
}

void * str_dict_get(struct dict * h, char * key) {
  struct dict_entry * e = str_lookup(h, key);
  if (e) {
    return e -> value;
  } else {
    return NULL;
  }
}

void str_dict_delete(struct dict * h, char * key) {
  if (str_lookup(h, key)) {
    // Key has to exist to be deleted
    struct dict_entry * entry , * temp;
    if ((entry = h -> hashtab[str_hash(h, key)]) && (h -> hashtab[str_hash(h, key)]) -> key == key) {
      // First element in the hashtab
      h -> hashtab[str_hash(h, key)] = entry -> next;
      return;
    }
    for (entry = (h -> hashtab[str_hash(h, key)]); entry -> next != NULL; entry = entry -> next) {
      if (entry -> next -> key == key) {
        // Found the entry: delete it
        entry -> next = temp -> next ;
      }
    }
  }
}

struct dict * str_dict_merge(struct dict * mergee, struct dict * merger) {
  struct dict_entry * entry;
  unsigned int index;
  // Create a copy of the mergee
  struct dict * result = str_dict_copy(mergee);
  // Look through each key of the merger, add it to the result
  for (index = 0; index < merger -> hashsize; index ++) {
    entry = merger -> hashtab[index];
    while (entry != NULL) {
      // Found a key value pair
      // Add it to the new str_dict
      str_dict_set(result, entry -> key, entry -> value);
      entry = entry -> next;
    }
  }
  return result;
}

struct dict * str_dict_copy(struct dict * h) {
  struct dict * result = str_dict_new(h -> hashsize);
  struct dict_entry * entry;
  unsigned int index;
  // Look through each key of the mergee, add it to the result
  for (index = 0; index < h -> hashsize; index ++) {
    entry = h -> hashtab[index];
    while (entry != NULL) {
      // Found a key value pair
      // Add it to the new str_dict
      str_dict_set(result, entry -> key, entry -> value);
      entry = entry -> next;
    }
  }
  return result;
}

#endif
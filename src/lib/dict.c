/*
  dict.c
  Basic library underlying the the attribute system of all objects.
  Uses a hash to store key-value pairs
    Keys: void * (addresses of memory, expected to be Iridium Atoms)
    Values: void * (pointer to any Iridium Object)
*/

#include "dict.h"

struct dict * dict_new(unsigned int hashsize) {
  struct dict * h;
  h = GC_MALLOC(sizeof(struct dict));
  assert(h);
  h -> hashsize = hashsize;
  h -> hashtab = (struct dict_entry **) GC_MALLOC(hashsize * sizeof(struct dict_entry *));
  assert(h -> hashtab);
  return h;
}

struct dict_entry * dict_set_base(struct dict * h, void * key) {
  struct dict_entry * entry ;
  // associate key with value in hash
  if (!(entry = lookup(h, key))) {
    // not in the hash
    entry = (struct dict_entry *) GC_MALLOC(sizeof(struct dict_entry));
    assert(entry); // Ensure that entry has been allocated.
    entry -> next = (h -> hashtab)[hash(h, key)]; // Make the head of the entry linked list
    (h -> hashtab)[hash(h, key)] = entry; // Make this the first element seen when looked up in the hashtab array.
  }
  entry -> key = key;
  return entry;
}

void dict_set(struct dict * h, void * key, void * value) {
  struct dict_entry * entry = dict_set_base(h, key);
  (entry -> value).ptr = value;
}

void dict_set_integral(struct dict * h, void * key, long long int value) {
  struct dict_entry * entry = dict_set_base(h, key);
  (entry -> value).integral = value;
}

void dict_set_flt(struct dict * h, void * key, double value) {
  struct dict_entry * entry = dict_set_base(h, key);
  (entry -> value).flt = value;
}

struct dict * dict_with(struct dict * h, void * key, void * value) {
  struct dict * result = dict_copy(h);
  dict_set(result, key, value);
  return result;
}

struct dict_entry * lookup(struct dict * h, void * key) {
  // Find the dict_entry element corresponding to a key
  // Return NULL if key not present
  struct dict_entry * entry ;
  for (entry = (h -> hashtab[hash(h, key)]); entry != NULL; entry = entry -> next)
    if (key == entry -> key)
      return entry; // Found the entry with key: key
  return NULL; // No entries found
}

void * dict_get(struct dict * h, void * key) {
  struct dict_entry * e = lookup(h, key);
  if (e) {
    return (e -> value).ptr;
  } else {
    return NULL;
  }
}

long long int dict_get_integral(struct dict * h, void * key) {
  struct dict_entry * e = lookup(h, key);
  if (e) {
    return (e -> value).integral;
  } else {
    return 0;
  }
}

double dict_get_flt(struct dict * h, void * key) {
  struct dict_entry * e = lookup(h, key);  
  if (e) {
    return (e -> value).flt;
  } else {
    return NAN;
  }
}

void dict_delete(struct dict * h, void * key) {
  if (lookup(h, key)) {
    // Key has to exist to be deleted
    struct dict_entry * entry;
    if ((entry = h -> hashtab[hash(h, key)]) && (h -> hashtab[hash(h, key)]) -> key == key) {
      // First element in the hashtab
      h -> hashtab[hash(h, key)] = entry -> next;
      return;
    }
    for (entry = (h -> hashtab[hash(h, key)]); entry -> next != NULL; entry = entry -> next) {
      if (entry -> next -> key == key) {
        // Found the entry: delete it
        entry -> next = entry -> next -> next;
      }
    }
  }
}

struct dict * dict_merge(struct dict * mergee, struct dict * merger) {
  struct dict_entry * entry;
  unsigned int index;
  // Create a copy of the mergee
  struct dict * result = dict_copy(mergee);
  // Look through each key of the merger, add it to the result
  for (index = 0; index < merger -> hashsize; index ++) {
    entry = merger -> hashtab[index];
    while (entry != NULL) {
      // Found a key value pair
      // Add it to the new dict
      dict_set(result, entry -> key, (entry -> value).ptr);
      entry = entry -> next;
    }
  }
  return result;
}

struct dict * dict_copy(struct dict * h) {
  struct dict * result = dict_new(h -> hashsize);
  struct dict_entry * entry;
  unsigned int index;
  // Look through each key of the mergee, add it to the result
  for (index = 0; index < h -> hashsize; index ++) {
    entry = h -> hashtab[index];
    while (entry != NULL) {
      // Found a key value pair
      // Add it to the new dict
      dict_set(result, entry -> key, (entry -> value).ptr);
      entry = entry -> next;
    }
  }
  return result;
}

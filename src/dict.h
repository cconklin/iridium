/*
  dict.h
  Basic library underlying the Dictionary class and the attribute system of all objects.
  Uses a hash to store key-value pairs
    Keys: void * (addresses of memory, expected to by Iridium Atoms)
    Values: void * (pointer to any Iridium Object)
*/

#include <stdlib.h>
#include <assert.h>
#include <gc.h>

#define hash(h, key) ((unsigned int) key) % h -> hashsize

struct dict_entry {
  struct dict_entry * next;
  void * key; // Use an address of memory as the key
  void * value; // Store some arbitrary data
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

struct dict * dict_new(unsigned int hashsize) {
  struct dict * h;
  h = GC_MALLOC(sizeof(struct dict));
  assert(h);
  h -> hashsize = hashsize;
  h -> hashtab = (struct dict_entry **) GC_MALLOC(hashsize * sizeof(struct dict_entry *));
  assert(h -> hashtab);
  return h;
}

void dict_set(struct dict * h, void * key, void * value) {
  struct dict_entry * entry ;
  // associate key with value in hash
  if (!(entry = lookup(h, key))) {
    // not in the hash
    entry = (struct dict_entry *) GC_MALLOC(sizeof(struct dict_entry));
    assert(entry); // Ensure that entry has been allocated.
    entry -> next = (h -> hashtab)[hash(h, key)]; // Make the head of the entry linked list
    (h -> hashtab)[hash(h, key)] = entry; // Make this the first element seen when looked up in the hashtab array.
  }
  entry -> value = value;
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
    return e -> value;
  } else {
    return NULL;
  }
}

void dict_delete(struct dict * h, void * key) {
  if (lookup(h, key)) {
    // Key has to exist to be deleted
    struct dict_entry * entry , * temp;
    if ((entry = h -> hashtab[hash(h, key)]) && (h -> hashtab[hash(h, key)]) -> key == key) {
      // First element in the hashtab
      h -> hashtab[hash(h, key)] = entry -> next;
      return;
    }
    for (entry = (h -> hashtab[hash(h, key)]); entry -> next != NULL; entry = entry -> next) {
      if (entry -> next -> key == key) {
        // Found the entry: delete it
        entry -> next = temp -> next ;
      }
    }
  }
}

struct dict * dict_merge(struct dict * mergee, struct dict * merger) {
  struct dict * result = dict_new(mergee -> hashsize);
  struct dict_entry * entry;
  unsigned int index;
  // Look through each key of the mergee, add it to the result
  for (index = 0; index < mergee -> hashsize; index ++) {
    entry = mergee -> hashtab[index];
    while (entry != NULL) {
      // Found a key value pair
      // Add it to the new dict
      dict_set(result, entry -> key, entry -> value);
      entry = entry -> next;
    }
  }
  // Look through each key of the merger, add it to the result
  for (index = 0; index < merger -> hashsize; index ++) {
    entry = merger -> hashtab[index];
    while (entry != NULL) {
      // Found a key value pair
      // Add it to the new dict
      dict_set(result, entry -> key, entry -> value);
      entry = entry -> next;
    }
  }
  return result;
}
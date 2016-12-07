#include "object.h"

#ifndef IRIDIUM_DICTIONARY
#define IRIDIUM_DICTIONARY

// Closest value to 4k
#define IR_DICTIONARY_HASHSIZE 4093

struct IR_DICTIONARY_ENTRY {
  struct IR_DICTIONARY_ENTRY * ordered_next; // IR dictionaries are ordered, this holds the element added after this
  struct IR_DICTIONARY_ENTRY * ordered_prev; // IR dictionaries are ordered, this holds the element added before this
  struct IR_DICTIONARY_ENTRY * next; // Linked list used for hash collisions
  object key;
  object value;
};

struct IR_DICTIONARY {
  struct IR_DICTIONARY_ENTRY ** hashtable;
  struct IR_DICTIONARY_ENTRY * first;
  struct IR_DICTIONARY_ENTRY * last;
};

struct IR_DICTIONARY * new_IR_DICTIONARY(void) {
  struct IR_DICTIONARY * dict = GC_MALLOC(sizeof(struct IR_DICTIONARY));
  assert(dict);
  dict -> hashtable = GC_MALLOC(sizeof(struct IR_DICTIONARY_ENTRY *) * IR_DICTIONARY_HASHSIZE);
  assert(dict -> hashtable);
  memset(dict -> hashtable, 0, sizeof(struct IR_DICTIONARY_ENTRY *) * IR_DICTIONARY_HASHSIZE);
  dict -> first = NULL;
  dict -> last = NULL;
  return dict;
}

struct IR_DICTIONARY_ENTRY * lookup_IR_DICTIONARY(struct IR_DICTIONARY * dict, object key, struct IR_DICTIONARY_ENTRY ** prev) {
  unsigned long long int hash = (unsigned long long int) INT(send(key, "hash"));
  unsigned long long int searchval = hash % IR_DICTIONARY_HASHSIZE;
  struct IR_DICTIONARY_ENTRY * entry = dict -> hashtable[searchval];
  * prev = NULL;
  while (NULL != entry) {
    if (ir_cmp_true == send(key, "__eq__", entry -> key)) {
      return entry;
    }
    * prev = entry;
    entry = entry -> next;
  }
  return NULL;
}

void insert_IR_DICTIONARY(struct IR_DICTIONARY * dict, object key, object value) {
  struct IR_DICTIONARY_ENTRY * prev;
  struct IR_DICTIONARY_ENTRY * new_entry;
  struct IR_DICTIONARY_ENTRY * entry = lookup_IR_DICTIONARY(dict, key, &prev);
  unsigned long long int hash = (unsigned long long int) INT(send(key, "hash"));
  unsigned long long int searchval = hash % IR_DICTIONARY_HASHSIZE;
  if (NULL == entry) {
    // Create the new element
    new_entry = GC_MALLOC(sizeof(struct IR_DICTIONARY_ENTRY));
    assert(new_entry);
    // Initialize the new entry
    new_entry -> next = NULL;
    new_entry -> ordered_next = NULL;
    new_entry -> ordered_prev = dict -> last;
    new_entry -> key = key;
    new_entry -> value = value;
    if (dict -> last == NULL) {
      dict -> last = new_entry;
    } else {
      dict -> last -> ordered_next = new_entry;
    }
    dict -> last = new_entry;
    if (dict -> first == NULL) {
      dict -> first = new_entry;
    }
    // New addition to dictionary
    if (prev == NULL) {
      dict -> hashtable[searchval] = new_entry;
    } else {
      // Entry is now the last in it's chain
      prev -> next = new_entry;
    }
  } else {
    // Overwrite old value
    entry -> value = value;
  }
}

// returns -1 if not present, 0 on success
int remove_IR_DICTIONARY(struct IR_DICTIONARY * dict, object key) {
  struct IR_DICTIONARY_ENTRY * prev;
  struct IR_DICTIONARY_ENTRY * entry = lookup_IR_DICTIONARY(dict, key, &prev);
  unsigned long long int hash = (unsigned long long int) INT(send(key, "hash"));
  unsigned long long int searchval = hash % IR_DICTIONARY_HASHSIZE;
  if (NULL != entry) {
    // Present
    if (prev == NULL) {
      // First in the hashtab
      dict -> hashtable[searchval] = NULL;
    } else {
      // Remove from the linked list
      prev -> next = entry -> next;
    }
    // Remove in order
    if (entry -> ordered_prev != NULL) {        
      entry -> ordered_prev -> ordered_next = entry -> ordered_next;
    }
    if (entry -> ordered_next != NULL) {        
      entry -> ordered_next -> ordered_prev = entry -> ordered_prev;
    }
    // If first/last, set to the next/previous
    if (entry == dict -> first) {
      dict -> first = entry -> ordered_next;
    }
    if (entry == dict -> last) {
      dict -> last = entry -> ordered_prev;
    }
    return 0;
  } else {
    // Not present
    return -1;
  }
}

// Class name
object CLASS(Dictionary);

iridium_method(Lambda, dict_initialize) {
  object elem = local("element");
  object acc = local("acc");
  object key = send(elem, "__get_index__", FIXNUM(0));
  object value = send(elem, "__get_index__", FIXNUM(1));
  // Hooray for breaking type safety...
  struct IR_DICTIONARY * dict = (struct IR_DICTIONARY *) acc;
  insert_IR_DICTIONARY(dict, key, value);
  return acc;
}

// Dictionary#initialize
// Creates a dictionary
// Dictionary.new([{:a, 1}, {:b, 2}, {"c", 3}]) # => %{:a => 1, :b => 2, "c" => 3}
// Args:
//    elements (list of tuples)
iridium_method(Dictionary, initialize) {
  object self = local("self"); // Receiver
  object args = local("elements"); // List of tuples
  object reduce_fn = FUNCTION(ATOM("lambda"), ARGLIST(argument_new(ATOM("element"), NULL, 0), argument_new(ATOM("acc"), NULL, 0)), dict_new(ObjectHashsize), iridium_method_name(Lambda, dict_initialize));
  struct IR_DICTIONARY * dict = new_IR_DICTIONARY();
  send(args, "reduce", (object) dict, reduce_fn);
  internal_set_attribute(self, ATOM("dict"), dict);
  return NIL;
}

// Dictionary#inspect
// String representation of dictionary
// Ignores possiblity of containing itself as a value/key
iridium_method(Dictionary, inspect) {
  object self = local("self");
  struct IR_DICTIONARY * dict = internal_get_attribute(self, ATOM("dict"), struct IR_DICTIONARY *);
  object str = IR_STRING("%{");
  struct IR_DICTIONARY_ENTRY * entry = dict -> first;
  while (entry) {      
    if (entry != dict -> first) {        
      str = send(str, "__add__", IR_STRING(", "));
    }
    str = send(str, "__add__", _send(entry -> key, "inspect", 0));
    str = send(str, "__add__", IR_STRING(" => "));
    str = send(str, "__add__", _send(entry -> value, "inspect", 0));
    entry = entry -> ordered_next;
  }
  return send(str, "__add__", IR_STRING("}"));
}

// Dictionary Init
void IR_init_Dictionary() {
  CLASS(Dictionary) = send(CLASS(Class), "new", IR_STRING("Dictionary"));
  DEF_METHOD(CLASS(Dictionary), "initialize", ARGLIST(argument_new(ATOM("elements"), NULL, 0)), iridium_method_name(Dictionary, initialize));
  DEF_METHOD(CLASS(Dictionary), "inspect", ARGLIST(), iridium_method_name(Dictionary, inspect));
}

#endif


#include "ir_object.h"

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

// Class name
object CLASS(Dictionary);

// Dictionary Init
void IR_init_Dictionary(struct IridiumContext *);

#endif

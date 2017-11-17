/* For copyright information, see olden_v1.0/COPYRIGHT */

#include<stdio.h>

typedef struct hash_entry {
  unsigned int key;
  int entry;
  struct hash_entry *next;
} *HashEntry;

typedef struct hash {
  HashEntry *array;
  int (*mapfunc)(unsigned int);
  int size;
} *Hash;

Hash MakeHash(int size, int (*map)(unsigned int));
int HashLookup(unsigned int key, Hash hash);
void HashInsert(int entry,unsigned int key, Hash hash);
void HashDelete(unsigned int key, Hash hash);


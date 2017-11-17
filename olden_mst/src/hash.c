/* For copyright information, see olden_v1.0/COPYRIGHT */

#include"hash.h"
#include"capability.h"
#include<cheric.h>
#include<string.h>
#include<sys/mman.h>

#define assert(num,a) if (!(a)) {printf("Assertion failure:%d in hash\n",num); exit(-1);}
#undef assert
#define assert(num,a)

int remaining = 0;
char* temp;

char *localmalloc(int size) 
{
  char *blah;
  
  if (size>remaining) 
    {
      temp = mmap(NULL, 64<<20, PROT_RW, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
      if(temp == MAP_FAILED) printf("Error! malloc returns null\n");
      remaining = 64<<20;
    }
  blah = temp;
  temp += size;
  remaining -= size;
  return cheri_setbounds(blah, size);
}

#define localfree(sz)

Hash MakeHash(int size, int (*map)(unsigned int)) 
{
  Hash retval;
  void* cp;

  /* modified by jqwu@princeton, use the library malloc */
  retval = (Hash)localmalloc(sizeof(*retval)); 
  cp = localmalloc(size*sizeof(HashEntry)); /* was localmalloc */
  memset(cp, 0, size * sizeof(HashEntry));
  retval->array = (HashEntry*)cp;
  retval->mapfunc = map;
  retval->size = size;
  return retval;
}

int HashLookup(unsigned int key, Hash hash)
{
  int j;
  HashEntry ent;

  j = (hash->mapfunc)(key);
  assert(1,j>=0);
  assert(2,j<hash->size);

  for (ent = hash->array[j]; ent && ent->key!=key; ent=ent->next) ;

  if (ent) return ent->entry;
  return 0;
}

void HashInsert(int entry,unsigned int key,Hash hash) 
{
  HashEntry ent;
  int j;
  
  assert(3,!HashLookup(key,hash));
  
  j = (hash->mapfunc)(key);
  
  /* modified by jqwu */
  ent = (HashEntry) localmalloc(sizeof(*ent));
  ent->next = hash->array[j];
  hash->array[j]=ent;
  ent->key = key;
  ent->entry = entry;
}

void HashDelete(unsigned int key,Hash hash)
{
  HashEntry *ent;
  HashEntry tmp;
  int j;

  j = (hash->mapfunc)(key);
  for (ent=(HashEntry*__capability)&(hash->array[j]);
    CAP_VALID(*ent) &&
      (*ent)->key!=key; ent=(HashEntry*__capability)&((*ent)->next))
      ;
  assert(4,*ent);
  tmp = *ent;
  *ent = (*ent)->next;
  localfree(tmp);
}

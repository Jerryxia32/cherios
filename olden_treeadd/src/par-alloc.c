/* For copyright information, see olden_v1.0/COPYRIGHT */

/* tree-alloc.c
 */

#include<mips.h>
#include<cheric.h>
#include"tree.h"
#include<sys/mman.h>
#include<stdio.h>

size_t remaining = 0;
char* temp;

static void __attribute__((noinline))
mallocinit() {
  temp = mmap(NULL, 64<<20, PROT_RW, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  if(temp == MAP_FAILED) printf("Error! malloc returns null\n");
  remaining = 64<<20;
}

char *localmalloc(size_t size) 
{
  char *blah;
  
  if(size>remaining) mallocinit();
  blah = temp;
  temp += size;
  remaining -= size;
  return cheri_setbounds(blah, size);
}

tree_t *TreeAlloc (lvl)
    int		lvl;
{
    /*
  register_t sp;
  __asm__ __volatile__ ("move %0, $sp" : "=r" (sp));
  printf("Stack pointer 0x%llx\n", sp);
   */
  if (lvl == 0)
    {
      return NULL;
    }
  else 
    {
      struct tree *new, *right, *left;
      
      new = (struct tree *)localmalloc(sizeof(tree_t));
      left = TreeAlloc(lvl-1);
      right=TreeAlloc(lvl-1);
      new->val = 1;
      new->left = (struct tree *) left;
      new->right = (struct tree *) right;
      return new;
    }
}

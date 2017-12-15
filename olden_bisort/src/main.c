/* JWS */

#include<mips.h>
#include<cheric.h>
#include<stdio.h>
#include<stdlib.h>
#include<statcounters.h>
#include<olden_config.h>
#include<sys/mman.h>
#define random OL_random

uint32_t random(uint32_t);
extern int dealwithargs (int argc, char *argv[]);

/* SWJ */

/* For copyright information, see olden_v1.0/COPYRIGHT */

/* =================== PROGRAM bitonic===================== */
/* UP - 0, DOWN - 1 */

#include "node.h"   /* Node Definition */
#include "proc.h"   /* Procedure Types/Nums */

#define CONST_m1 10000
#define CONST_b 31415821
#define RANGE 100

#define put(a) printf("%d",a)
#define puts(a) printf(a)


int flag=0,foo=0;
void*__capability malloc_pool;

void*
own_malloc(size_t n) {
    void* ret = cheri_setbounds(malloc_pool, n);
    malloc_pool = cheri_incoffset(malloc_pool, n);
    return ret;
}

#define NewNode(h,v) \
  \
{ \
    h = (HANDLE *) own_malloc(sizeof(struct node)); \
      h->value = v; \
	h->left = NIL; \
	  h->right = NIL; \
	  };


void InOrder(h)
     HANDLE *h;
{
  HANDLE *l, *r;
  if ((h != NIL))
    {
      l = h->left;
      r = h->right;
      InOrder(l);
      printf("%d @ 0x%x\n",h->value,
	       0 /* h */);
      /* JWS: normalize output */
      InOrder(r);
    }
}

int mult(int p, int q)
{
	int p1, p0, q1, q0;
	
	p1=p/CONST_m1; p0=p%CONST_m1;
	q1=q/CONST_m1; q0=q%CONST_m1;
	return (((p0*q1+p1*q0) % CONST_m1)*CONST_m1+p0*q0);
}

uint32_t skiprand(uint32_t seed, uint32_t n)
{
  for (; n; n--) seed=random(seed);
  return seed;
}

uint32_t random(uint32_t seed)
{
  int tmp;
  tmp = (mult(seed,CONST_b)+1);
  return tmp;
}

HANDLE* RandTree(n,seed,level)
     int n,seed,level;

{
  int next_val,my_name;
  HANDLE *h;
  my_name=foo++;

  if (n > 1)
    {
      seed = random(seed);
      next_val=seed % RANGE;
      NewNode(h,next_val);

      h->left = RandTree((n/2),seed,level+1);
      h->right =  RandTree((n/2),skiprand(seed,(n)+1),level+1);
    }
  else 
    h = NIL;
  return(h);
} 

void SwapVal(l,r)
     HANDLE *l, *r;
{ 
  int temp;
  
  temp = l->value; /* MISS PROBLEM */
  l->value = r->value;
  r->value = temp;
} 

void SwapLeft(l,r)
     HANDLE *l, *r;
{
  HANDLE *h;

  h = r->left;
  r->left = l->left;
  l->left = h;
}

void SwapRight(l,r)
     HANDLE *l, *r;
{  
  HANDLE *h;

  h = r->right;
  r->right = l->right; /* MISS PROBLEM */
  l->right = h;
} 

int Bimerge(root,spr_val,dir)
     HANDLE *root;
     int spr_val,dir;
{ 
  int rightexchange, elementexchange;
  HANDLE *pl, *pr;

  /*chatting("enter bimerge %x\n", root);*/
  rightexchange = ((root->value > spr_val) ^ dir);
  if (rightexchange)
    {
      int temp;
      temp = root->value;
      root->value = spr_val;
      spr_val = temp;
    }
  
  pl = root->left;
  pr = root->right;

  while (pl != NIL)
    {
      elementexchange = ((pl->value > /* MISS PROBLEM */pr->value) ^ dir); 
      if (rightexchange)
	{
	  if (elementexchange)
	    { 
	      SwapVal(pl,pr);
	      SwapRight(pl,pr);
	      pl = pl->left;
	      pr = pr->left;
	    }
	  else 
	    { 
	      pl = pl->right;
	      pr = pr->right;
	    }
	}
      else 
	{
	  if (elementexchange)
	    { 
	      SwapVal(pl,pr);
	      SwapLeft(pl,pr);
	      pl = pl->right;
	      pr = pr->right;
	    }
	  else 
	    { 
	      pl = pl->left;
	      pr = pr->left; /* MISS PROBLEM */
	    }
	}
    }

  if (root->left != NIL)
    { 
      root->value=Bimerge(root->left,root->value,dir);
      spr_val=Bimerge(root->right,spr_val,dir);
    }
  /*chatting("exit bimerge %x\n", root);*/
  return spr_val;
} 

int Bisort(root,spr_val,dir)
     HANDLE *root;
     int spr_val,dir;
{ 
  /*chatting("bisort %x\n", root);*/
  if (root->left == NIL)
    { 
     if ((root->value > spr_val) ^ dir)
        {
	  int val;
	  val = spr_val;
	  spr_val = root->value;
	  root->value =val;
	}
    }
  else 
    {
      /* Bisort both halves of the tree and merge */
      root->value=Bisort(root->left,root->value,dir);
      spr_val=Bisort(root->right,spr_val,!dir);
      spr_val=Bimerge(root,spr_val,dir);
    }
  /*chatting("exit bisort %x\n", root);*/
  return spr_val;
} 

int main(argc,argv)
  int argc;
 char *argv[];


{ 
  malloc_pool = mmap(NULL, 256<<20, PROT_RW, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  stats_init();
  HANDLE *h;
  int sval;
  int n;
   
  n = dealwithargs(argc,argv);

  for(int i=0; i<OLDEN_BISORT_ITER; i++) {
    malloc_pool = cheri_setoffset(malloc_pool, 0);
    printf("Bisort with %d size\n", n);
    h = RandTree(n,12345768,0);
    sval = random(245867) % RANGE;
    if (flag) {
      InOrder(h);
      printf("%d\n",sval);
     }
    printf("**************************************\n");
    printf("BEGINNING BITONIC SORT ALGORITHM HERE\n");
    printf("**************************************\n");

    printf("Sorting forward...");
    sval=Bisort(h,sval,0);
    if (flag) {
      printf("Sorted Tree:\n"); 
      InOrder(h);
      printf("%d\n",sval);
     }
    printf("done\n");

    printf("sorting backward...");
    sval=Bisort(h,sval,1);
    if (flag) {
      printf("Sorted Tree:\n"); 
      InOrder(h);
      printf("%d\n",sval);
     }
    printf("done\n");
  }
  malloc_pool = cheri_setoffset(malloc_pool, 0);
  stats_display();
  munmap(malloc_pool, 256<<20);
  return(0); /* just to get rid of the warnning */
} 


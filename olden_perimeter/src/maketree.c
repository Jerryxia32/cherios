/* For copyright information, see olden_v1.0/COPYRIGHT */

#include"perimeter.h"
#include<sys/mman.h>
#include<stdio.h>
#include<cheric.h>

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

static int CheckOutside(int x, int y) 
{
  int euclid = x*x+y*y;

  if (euclid > 4194304) return 1;  
  if (euclid < 1048576) return -1; 
  return 0;
}

static int CheckIntersect(int center_x, int center_y, int size)
{
  int sum;
  
  if (!CheckOutside(center_x+size,center_y+size) &&
      !CheckOutside(center_x+size,center_y-size) &&
      !CheckOutside(center_x-size,center_y-size) &&
      !CheckOutside(center_x-size,center_y+size)) return 2;
  sum=CheckOutside(center_x+size,center_y+size) +
    CheckOutside(center_x+size,center_y-size) +
      CheckOutside(center_x-size,center_y-size) +
	CheckOutside(center_x-size,center_y+size);
  if ((sum==4) || (sum==-4)) return 0;
  return 1;
}  

QuadTree MakeTree(int size, int center_x, int center_y, 
		  QuadTree parent, ChildType ct, int level) 
{
  int intersect=0;
  QuadTree retval;

  retval = (QuadTree)localmalloc(sizeof(*retval));
  
  retval->parent = parent;
  retval->childtype = ct;
  retval->nw = retval->ne = retval->sw = retval->se = NULL;
 
  intersect = CheckIntersect(center_x,center_y,size);
  size = size/2;
  if ((intersect==0) && (size<2/*512*/))
    {
      retval->color = white;
    }
  else if (intersect==2) 
    {
      retval->color=black;
    }
  else 
    {
      if (!level)
	{
	  retval->color = black;
	}
      else 
	{
	  /* These are allocated in this order, make sure they are 
	     traversed in this order */
	  retval->nw = MakeTree(size,center_x-size,center_y+size,
				retval,northwest,level-1);
	  retval->ne = MakeTree(size,center_x+size,center_y+size,
				retval,northeast,level-1);
	  retval->se = MakeTree(size,center_x+size,center_y-size,
				retval,southeast,level-1);
	  retval->sw = MakeTree(size,center_x-size,center_y-size,
				retval,southwest,level-1);
	  retval->color = grey;
	}
    }
  return retval;
}

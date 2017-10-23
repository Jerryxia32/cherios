/* JWS */

#include<olden_config.h>
#include<stdlib.h>

/* SWJ */

/* For copyright information, see olden_v1.0/COPYRIGHT */

int mylog(int num)
{
  int j=0,k=1;
  
  while(k<num) { k*=2; j++; }
  return j;
} 

extern int flag;
int dealwithargs(int argc, char *argv[])
{
  int size;

  flag = OLDEN_BISORT_FLAG;
  size = OLDEN_BISORT_SIZE;

  return size;  
}


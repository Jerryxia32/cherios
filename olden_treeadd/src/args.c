/* For copyright information, see olden_v1.0/COPYRIGHT */

#include"tree.h"
#include<olden_config.h>

int level; 
int iters;

int dealwithargs(int argc, char *argv[])
{
  iters = OLDEN_TREEADD_ITER;
  level = OLDEN_TREEADD_LEVEL;

  return level;
}

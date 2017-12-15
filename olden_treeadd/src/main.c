/* For copyright information, see olden_v1.0/COPYRIGHT */

/* node.c
 */
#include<mips.h>
#include<cheric.h>
#include"tree.h"
#include<stdio.h>
#include<statcounters.h>

int dealwithargs(int argc, char *argv[]);

typedef struct {
    long 	level;
} startmsg_t;

int TreeAdd (tree_t *t);
extern tree_t *TreeAlloc (int lvl);

extern size_t remaining;
extern char* temp;

int main (int argc, char *argv[])
{
    tree_t	*root;
    int i, result = 0;

    (void)dealwithargs(argc, argv);

    stats_init();
    for (i = 0; i < iters; i++) 
      {
    printf("Treeadd with %d levels\n", level);

    printf("About to enter TreeAlloc\n"); 
    root = TreeAlloc (level);
    printf("About to enter TreeAdd\n"); 
    
	fprintf(stderr, "Iteration %d...", i);
	result = TreeAdd (root);
	fprintf(stderr, "done\n");

    printf("Received result of %d\n",result);
    remaining = 256<<20;
    temp = cheri_setoffset(temp, 0);
      }
    stats_display();
    return 0;
}

/* TreeAdd:
 */
int TreeAdd (t)
     register tree_t	*t;
{
  if (t == NULL)  
    {
      return 0;
    }
  else 
    {
      int leftval;
      int rightval;
      tree_t *tleft, *tright;
      int value;
      
      tleft = t->left;
      leftval = TreeAdd(tleft);
      tright = t->right;
      rightval = TreeAdd(tright);

      value = t->val;
      return leftval + rightval + value;
    }
} /* end of TreeAdd */

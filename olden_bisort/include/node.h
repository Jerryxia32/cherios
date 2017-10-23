/* For copyright information, see olden_v1.0/COPYRIGHT */

/* =============== NODE STRUCTURE =================== */

struct node { 
  struct node *left;
  struct node *right;
  int value;
};

typedef struct node HANDLE;

#define NIL ((HANDLE *) 0)
#ifdef FUTURES
#define makenode(procid) ALLOC(procid,sizeof(struct node))
#else
#define makenode(procid) mymalloc(sizeof(struct node))
#endif

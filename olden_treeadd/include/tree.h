/* For copyright information, see olden_v1.0/COPYRIGHT */

/* tree.h
 */

typedef struct tree {
    int		val;
    struct tree *left, *right;
} tree_t;

extern tree_t *TreeAlloc (/*int level*/);

extern int level;
extern int iters;

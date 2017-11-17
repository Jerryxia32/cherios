/* For copyright information, see olden_v1.0/COPYRIGHT */

#ifdef SS_PLAIN
#include "ssplain.h"
#endif //SS_PLAIN 

typedef enum {black, white, grey} Color;
typedef enum {northwest, northeast, southwest, southeast} ChildType;
typedef enum {north, east, south, west} Direction;

typedef struct quad_struct {
  struct quad_struct *nw;
  struct quad_struct *ne;
  struct quad_struct *sw;
  struct quad_struct *se;
  struct quad_struct *parent;
  Color color;
  ChildType childtype;
} *QuadTree;

QuadTree MakeTree(int size, int center_x, int center_y, 
		  QuadTree parent, ChildType ct, int level);



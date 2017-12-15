/* For copyright information, see olden_v1.0/COPYRIGHT */

#include "hash.h"
#define MAXPROC 1

typedef struct vert_st {
  int mindist;
  struct vert_st *next;
  Hash edgehash;
  unsigned int padding;
} *Vertex;

typedef struct graph_st {
  Vertex vlist;
} *Graph;

Graph MakeGraph(int numvert);

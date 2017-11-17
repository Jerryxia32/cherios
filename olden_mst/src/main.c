/* For copyright information, see olden_v1.0/COPYRIGHT */

#include<string.h>
#include"mst.h"
#include<olden_config.h>
#include<statcounters.h>
#include<cheric.h>

typedef struct blue_return {
  Vertex vert;
  int dist;
} BlueReturn;

static BlueReturn BlueRule(Vertex inserted, Vertex vlist) 
{
  BlueReturn retval;
  Vertex tmp,prev;
  Hash hash;
  int dist,dist2;
  int count;
  
  if (!vlist) {
    retval.dist = 999999;
    return retval;
  }
  prev = vlist;
  retval.vert = vlist;
  retval.dist = vlist->mindist;
  hash = vlist->edgehash;
  dist = HashLookup((unsigned int) inserted, hash);
  /*printf("Found %d at 0x%x for 0x%x\n",dist,inserted,vlist);*/
  if (dist) 
    {
      if (dist<retval.dist) 
        {
          vlist->mindist = dist;
          retval.dist = dist;
        }
    }
  else printf("Not found\n");
  
  count = 0;

  /* We are guaranteed that inserted is not first in list */
  for (tmp=vlist->next; tmp; prev=tmp,tmp=tmp->next) 
    {
      count++;

      /* Splice chosen vertex out of the list */
      if (tmp==inserted) 
        {
          Vertex next;
          next = tmp->next;
          prev->next = next;
        }
      /* Find the shortest distance to any other vertex not in the list */
      else 
        {
          hash = tmp->edgehash;
          dist2 = tmp->mindist;
          dist = HashLookup((unsigned int) inserted, hash);
          /*printf("Found %d at 0x%x for 0x%x\n",dist,inserted,tmp);*/
          if (dist) 
            {
              if (dist<dist2) 
                {
                  tmp->mindist = dist;
                  dist2 = dist;
                }
            }
          else printf("Not found\n");
          if (dist2<retval.dist) 
            {
              retval.vert = tmp;
              retval.dist = dist2;
            }
        } /* else */

    } /* for */
  /*printf("Count was %d\n",count);*/
  return retval;
}

          

static Vertex MyVertexList = NULL;

static int ComputeMst(Graph graph,int numvert) 
{
  Vertex inserted,tmp;
  int cost=0,dist;

  /* make copy of graph */
  printf("Compute phase 1\n");

  /* Insert first node */
  inserted = graph->vlist;
  tmp = inserted->next;
  graph->vlist = tmp;
  MyVertexList = tmp;
  numvert--;

  /* Announce insertion and find next one */
  printf("Compute phase 2\n");
  while (numvert) 
    {
      BlueReturn br;

      if (inserted == MyVertexList)
	MyVertexList = MyVertexList->next;
      br = BlueRule(inserted, MyVertexList);
      inserted = br.vert;    
      dist = br.dist;
      numvert--;
      cost = cost+dist;
    }
  return cost;
}

extern int dealwithargs(int argc, char *argv[]);
extern int remaining;
extern char* temp;

int main(int argc, char *argv[]) 
{
  Graph graph;
  int dist;
  int size;
 
  printf("Hash entry size = %d\n", sizeof(struct hash_entry));
  printf("Hash size = %d\n", sizeof(struct hash));
  printf("Vertex size = %d\n", sizeof(struct vert_st));
  
  size = dealwithargs(argc,argv);

  stats_init();
  for(int i=0; i<OLDEN_MST_ITER; i++) {
    printf("Making graph of size %d\n",size);
    graph = MakeGraph(size);
    printf("Graph completed\n");

    printf("About to compute mst \n");
    dist = ComputeMst(graph,size);

    printf("MST has cost %d\n",dist);
    remaining = 64<<20;
    temp = cheri_setoffset(temp, 0);
  }
  stats_display();
  return 0;
}

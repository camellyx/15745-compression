/* For copyright information, see olden_v1.0/COPYRIGHT */

/* =================== PROGRAM bitonic===================== */
/* UP - 0, DOWN - 1 */
#include "node.h"   /* Node Definition */
#include "proc.h"   /* Procedure Types/Nums */
#include <stdio.h>
#include <assert.h>

#define CONST_m1 10000
#define CONST_b 31415821
#define RANGE 100

int NumNodes, NDim;

int random(int);

int flag=0,foo=0;

// splitting
#define NDEBUG
size_t MAX_COUNT = 0;
size_t count = 0;
HANDLE *node_arr; // equal to first field: value_arr
HANDLE *value_arr;
HANDLE **left_arr;
HANDLE **right_arr;

// splitting
#define LocalNewNode(h,v) \
{ \
  h = node_arr + count; \
  value_arr[count] = v; \
  left_arr[count] = NIL; \
  right_arr[count] = NIL; \
  count++; \
  assert(count < MAX_COUNT); \
};

#define NewNode(h,v,procid) LocalNewNode(h,v)

void InOrder(HANDLE *h) {
  HANDLE *l, *r;
  if ((h != NIL)) {
    size_t index = h - node_arr;
    l = left_arr[index];
    r = right_arr[index];
    InOrder(l);
    static unsigned char counter = 0;
    if (counter++ == 0)   /* reduce IO */
      printf("%d @ 0x%x\n",value_arr[index], 0);
    InOrder(r);
  }
}

int mult(int p, int q) {
  int p1, p0, q1, q0;

  p1 = p/CONST_m1; p0 = p%CONST_m1;
  q1 = q/CONST_m1; q0 = q%CONST_m1;
  return ((p0*q1+p1*q0) % CONST_m1)*CONST_m1+p0*q0;
}

/* Generate the nth random # */
int skiprand(int seed, int n) {
  for (; n; n--) seed=random(seed);
  return seed;
}

int random(int seed) {
  return mult(seed,CONST_b)+1;
}

HANDLE* RandTree(int n, int seed, int node, int level) {
  int next_val,my_name;
  future_cell_int f_left, f_right;
  HANDLE *h;
  my_name=foo++;
  if (n > 1) {
    int newnode;
    if (level<NDim)
      newnode = node + (1 <<  (NDim-level-1));
    else
      newnode = node;
    seed = random(seed);
    next_val=seed % RANGE;
    NewNode(h,next_val,node);
    f_left.value = RandTree((n/2),seed,newnode,level+1);
    f_right.value = RandTree((n/2),skiprand(seed,(n)+1),node,level+1);

    size_t index = h - node_arr;
    left_arr[index] = f_left.value;
    right_arr[index] = f_right.value;
  } else {
    h = NIL;
  }
  return h;
}

void SwapValue(HANDLE *l, HANDLE *r) {
  int temp,temp2;

  size_t lindex = l - node_arr;
  size_t rindex = r - node_arr;
  temp = value_arr[lindex];
  temp2 = value_arr[rindex];
  value_arr[rindex] = temp;
  value_arr[lindex] = temp2;
} 

void
/***********/
SwapValLeft(l,r,ll,rl,lval,rval)
  /***********/
  HANDLE *l;
  HANDLE *r;
  HANDLE *ll;
  HANDLE *rl;
  int lval, rval;
{
  size_t lindex = l - node_arr;
  size_t rindex = r - node_arr;
  value_arr[rindex] = lval;
  left_arr[rindex] = ll;
  left_arr[lindex] = rl;
  value_arr[lindex] = rval;
} 


void
/************/
SwapValRight(l,r,lr,rr,lval,rval)
  /************/
  HANDLE *l;
  HANDLE *r;
  HANDLE *lr;
  HANDLE *rr;
  int lval, rval;
{  
  size_t lindex = l - node_arr;
  size_t rindex = r - node_arr;
  value_arr[rindex] = lval;
  right_arr[rindex] = lr;
  right_arr[lindex] = rr;
  value_arr[lindex] = rval;
  /*printf("Swap Val Right l 0x%x,r 0x%x val: %d %d\n",l,r,lval,rval);*/
} 

int
/********************/
Bimerge(root,spr_val,dir)
  /********************/
  HANDLE *root;
  int spr_val,dir;

{ int rightexchange;
  int elementexchange;
  HANDLE *pl,*pll,*plr;
  HANDLE *pr,*prl,*prr;
  HANDLE *rl;
  HANDLE *rr;
  int rv,lv;


  /*printf("enter bimerge %x\n", root);*/
  size_t root_index = root - node_arr;
  rv = value_arr[root_index];

  pl = left_arr[root_index];
  pr = right_arr[root_index];
  rightexchange = ((rv > spr_val) ^ dir);
  if (rightexchange)
  {
    value_arr[root_index] = spr_val;
    spr_val = rv;
  }

  while ((pl != NIL))
  {
    /*printf("pl = 0x%x,pr = 0x%x\n",pl,pr);*/
    size_t pl_index = pl - node_arr;
    size_t pr_index = pr - node_arr;
    lv = value_arr[pl_index];        /* <------- 8.2% load penalty */
    pll = left_arr[pl_index];
    plr = right_arr[pl_index];       /* <------- 1.35% load penalty */
    rv = value_arr[pr_index];         /* <------ 57% load penalty */
    prl = left_arr[pr_index];         /* <------ 7.6% load penalty */
    prr = right_arr[pr_index];        /* <------ 7.7% load penalty */
    elementexchange = ((lv > rv) ^ dir);
    if (rightexchange)
      if (elementexchange)
      { 
        SwapValRight(pl,pr,plr,prr,lv,rv);
        pl = pll;
        pr = prl;
      }
      else 
      { pl = plr;
        pr = prr;
      }
    else 
      if (elementexchange)
      { 
        SwapValLeft(pl,pr,pll,prl,lv,rv);
        pl = plr;
        pr = prr;
      }
      else 
      { pl = pll;
        pr = prl;
      }
  }
  if ((left_arr[root_index] != NIL))
  { 
    int value;
    rl = left_arr[root_index];
    rr = right_arr[root_index];
    value = value_arr[root_index];

    value_arr[root_index]=Bimerge(rl,value,dir);
    spr_val=Bimerge(rr,spr_val,dir);
  }
  /*printf("exit bimerge %x\n", root);*/
  return spr_val;
} 

int
/*******************/
Bisort(root,spr_val,dir)
  /*******************/
  HANDLE *root;
  int spr_val,dir;

{ HANDLE *l;
  HANDLE *r;
  int val;
  /*printf("bisort %x\n", root);*/
  size_t root_index = root - node_arr;
  if ((left_arr[root_index] == NIL))  /* <---- 8.7% load penalty */
  { 
    if (((value_arr[root_index] > spr_val) ^ dir))
    {
      val = spr_val;
      spr_val = value_arr[root_index];
      value_arr[root_index] =val;
    }
  }
  else 
  {
    int ndir;
    l = left_arr[root_index];
    r = right_arr[root_index];
    val = value_arr[root_index];
    /*printf("root 0x%x, l 0x%x, r 0x%x\n", root,l,r);*/
    value_arr[root_index]=Bisort(l,val,dir);
    ndir = !dir;
    spr_val=Bisort(r,spr_val,ndir);
    spr_val=Bimerge(root,spr_val,dir);
  }
  /*printf("exit bisort %x\n", root);*/
  return spr_val;
} 

int main(int argc, char **argv) {
  HANDLE *h;
  int sval;
  int n;

  n = dealwithargs(argc,argv);

  // splitting
  MAX_COUNT = n;
  value_arr = (HANDLE*)malloc(sizeof(HANDLE) * MAX_COUNT);
  left_arr = (HANDLE**)malloc(sizeof(HANDLE*) * MAX_COUNT);
  right_arr = (HANDLE**)malloc(sizeof(HANDLE*) * MAX_COUNT);
  node_arr = value_arr;

  printf("Bisort with %d size of dim %d\n", n, NDim);

  h = RandTree(n,12345768,0,0);
  sval = random(245867) % RANGE;
  if (flag) {
    InOrder(h);
    printf("%d\n",sval);
  }
  printf("**************************************\n");
  printf("BEGINNING BITONIC SORT ALGORITHM HERE\n");
  printf("**************************************\n");

  sval=Bisort(h,sval,0);

  if (flag) {
    printf("Sorted Tree:\n"); 
    InOrder(h);
    printf("%d\n",sval);
  }

  sval=Bisort(h,sval,1);

  if (flag) {
    printf("Sorted Tree:\n"); 
    InOrder(h);
    printf("%d\n",sval);
  }

  return 0;
} 








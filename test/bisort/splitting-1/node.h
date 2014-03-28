/* For copyright information, see olden_v1.0/COPYRIGHT */

/* =============== NODE STRUCTURE =================== */

//virtual structure for pooling
struct node {
  //int index;
  int value;
  struct node *left;
  struct node *right;
};

typedef int HANDLE;
//typedef struct node HANDLE;

typedef struct future_cell_int{
  HANDLE *value;
} future_cell_int;

extern void *malloc(unsigned);

#define NIL ((HANDLE *) 0)

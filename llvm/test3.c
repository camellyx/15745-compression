#include <malloc.h>
#include <stdio.h>

struct test {int crap; char shit;};

int main () {

  struct test a[10];

  struct test test1;
  int c[10];
  int *d;
  
  struct test* b;
  test1.crap = 4;

  int i;
  for (i = 0; i < 10; i++) {
	a[i].crap=i;
	a[i].shit=i;
	c[i] = i;
  }

  b = a + test1.crap;
  d = c;

  b->crap = 20;

  int sum = 0;

  for (i = 0; i < 10; i++) {
	sum += a[i].crap;
	sum += a[i].shit;
	sum += d[i];
	sum += c[i];
  }

  printf("sum: %d\n", sum);

  return 0;

}

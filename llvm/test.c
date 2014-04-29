#include <malloc.h>

struct test {int crap; char shit;};

int main () {

//  struct test a[10];
  struct test* a = malloc(sizeof(struct test)*10);
  struct test test1;
  
  struct test* b;
  test1.crap = 4;

  int i;
  for (i = 0; i < 10; i++) {
	a[i].crap=i;
	a[i].shit=i;
//	c[i] = i;
  }


  int sum = 0;

  for (i = 0; i < 10; i++) {
	sum += a[i].crap;
	sum += a[i].shit;
//	sum += d[i];
//	sum += c[i];
  }

  return sum;

}

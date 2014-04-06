struct test {int crap; char dasdfas;};


int main () {

  struct test a[10];
  struct test test1;
  
  struct test* b;
  test1.crap = 4;

  int i;
  for (i = 0; i < 10; i++) {
	a[i].crap=i;
  }

  b = a + test1.crap;

  b->crap = 20;

  int sum = 0;

  for (i = 0; i < 10; i++) {
	sum += a[i].crap;
  }

  return sum;

}

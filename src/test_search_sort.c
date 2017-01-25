#include <stdio.h>
#include "generic_search.h"
#include "generic_sort.h"

int return_self(int x) {
  return x;
}

int return_negative(int x) {
  return -x;
}

int comp_int(int a, int b) {
  return b - a;
}

// defines search_integer
generic_search(integer, int, int)
// defines sort_integer
generic_sort(integer, int, int)

int main(int argc, char* argv[]) {
  int n, n_dat = 5;
  int a[] = {1, 9, 5, 7, 3};
  int idx[n_dat];
  int obj = 7;

  printf("original:\t");
  for(n = 0; n < n_dat; n++)
    printf("%d, ", a[n]);
  printf("\n");

  printf("search %d:\t", obj);
  n = search_integer(idx, a, obj, n_dat, return_self, comp_int);
  printf("%d found (at %d)\n", n, idx[0]);

  printf("sort descent:\t");
  sort_integer(a, n_dat, return_negative, comp_int);
  for(n = 0; n < n_dat; n++)
    printf("%d, ", a[n]);
  printf("\n");

  return 0;
}

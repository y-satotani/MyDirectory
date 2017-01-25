#include <stdio.h>
#include "date.h"

int main(int argc, char* argv[]) {
  struct date d;
  char d_str[] = "2000-1-1";
  char buf[10];
  parse_date(&d, d_str);

  dump_date_csv(buf, &d);
  printf("%s\n", buf);
  dump_date_fancy(buf, &d);
  printf("%s\n", buf);
  return 0;
}

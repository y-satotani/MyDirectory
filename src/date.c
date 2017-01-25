
#include "date.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mystring.h"

/************************
 * handling date
 ************************/
int parse_date(struct date* d, char* str) {
  char buf[strlen(str) + 1];
  char *ret[3], *e;
  int n;
  memcpy(buf, str, sizeof(buf));

  n = split(buf, ret, '-', 3);
  if(n != 3) return 0;
  d->y = strtol(ret[0], &e, 10);
  if(*e != '\0') return 0;
  d->m = strtol(ret[1], &e, 10);
  if(*e != '\0') return 0;
  d->d = strtol(ret[2], &e, 10);
  if(*e != '\0') return 0;
  return 1;
}

int dump_date_csv(char* ret, struct date* d) {
  return sprintf(ret, "%d-%d-%d", d->y, d->m, d->d);
}

int dump_date_fancy(char* ret, struct date* d) {
  return sprintf(ret, "%d/%d/%d", d->y, d->m, d->d);
}

void serialize_date(struct buffer* buf, struct date* d) {
  serialize_short(buf, d->y);
  serialize_short(buf, d->m);
  serialize_short(buf, d->d);
}

void deserialize_date(struct date* d, struct buffer* buf) {
  d->y = deserialize_short(buf);
  d->m = deserialize_short(buf);
  d->d = deserialize_short(buf);
}

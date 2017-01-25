#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "profile.h"

int main(int argc, char* argv) {
  struct buffer buf;
  struct date d = { 2017, 1, 17 }, e;
  struct profile p = { 12345, "newborn", d, "emacs", "misc text"}, q;
  char* str = malloc(128);
  parse_date(&d, "2017-01-17");

  new_buffer(&buf, 2);

  strcpy(str, "Hello World!");
  assert(serialize_str(&buf, str) == strlen("Hello World!") + 1 + sizeof(uint32_t));
  strcpy(str, "some string");
  assert(serialize_str(&buf, str) == strlen("some string") + 1 + sizeof(uint32_t));
  assert(serialize_date(&buf, &d) == sizeof(struct date));
  serialize_profile(&buf, &p);

  reset_buffer(&buf);

  assert(deserialize_str(&str, &buf) == strlen("Hello World!") + 1 + sizeof(uint32_t));
  assert(strcmp(str, "Hello World!") == 0);
  assert(deserialize_str(&str, &buf) == strlen("some string") + 1 + sizeof(uint32_t));
  assert(strcmp(str, "some string") == 0);
  assert(deserialize_date(&e, &buf) == sizeof(struct date));
  assert(e.y == 2017 && e.m == 1 && e.d == 17);
  deserialize_profile(&q, &buf);
  assert(q.id == 12345 && strcmp(q.name, "newborn") == 0 && strcmp(q.misc, "misc text") == 0);

  free(str);
  delete_profile(&q); // q is a profile from serialized data, so needs to be freed
  delete_buffer(&buf);
  return 0;
}


#include "mystring.h"
#include <stdlib.h>
#include <string.h>

/************************
 * string function
 ************************/
int split(char* str, char* ret[], char delim, int max) {
  int count = 0;
  ret[count++] = str;
  while(*str && count < max) {
    if(*str == delim) {
      *str = '\0';
      ret[count++] = str + 1;
    }
    str++;
  }
  return count;
}

void serialize_str(struct buffer* buf, char* str) {
  uint32_t size = strlen(str) + 1;
  serialize_long(buf, size);
  serialize_data(buf, str, size);
}

void deserialize_str(char** str, struct buffer* buf) {
  uint32_t size;
  size = deserialize_long(buf);
  *str = (char*) malloc(size);
  deserialize_data(*str, buf, size);
}

void deserialize_str_nocopy(char** str, struct buffer* buf) {
  uint32_t size;
  size = deserialize_long(buf);
  *str = buf->data + buf->next;
  buf->next += size;
}

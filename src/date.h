#pragma once

#include <stdint.h>
#include "buffer.h"

struct date {
  uint16_t y;
  uint16_t m;
  uint16_t d;
};

int parse_date(struct date* d, char* str);
int dump_date_csv(char* ret, struct date* d);
int dump_date_fancy(char* ret, struct date* d);
void serialize_date(struct buffer* buf, struct date* d);
void deserialize_date(struct date* d, struct buffer* buf);

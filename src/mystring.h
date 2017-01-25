#pragma once

#include "buffer.h"

int split(char* str, char* ret[], char delim, int max);
void serialize_str(struct buffer* buf, char* str);
void deserialize_str(char** str, struct buffer* buf);
void deserialize_str_nocopy(char** str, struct buffer* buf);

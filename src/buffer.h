#pragma once

#include <stdint.h>

struct buffer {
  void* data;
  uint32_t size;
  uint32_t next;
};

void new_buffer(struct buffer* buf, uint32_t init_buf_size);
void new_buffer_from_data(struct buffer* buf, uint32_t buf_size, void* data);
void reserve_space_for_buffer(struct buffer* buf, uint32_t additional);
void reset_buffer(struct buffer* buf);
void invalidate_buffer(struct buffer* buf);
void delete_buffer(struct buffer* buf);
int read_buffer(int sfd, struct buffer* buf, int bufsize);
int write_buffer(int sfd, struct buffer* buf, int bufsize);

void serialize_short(struct buffer* buf, short x);
short deserialize_short(struct buffer* buf);
void serialize_long(struct buffer* buf, long x);
long deserialize_long(struct buffer* buf);
void serialize_data(struct buffer* buf, void* data, uint32_t size);
void deserialize_data(void* data, struct buffer* buf, uint32_t size);


#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

/************************
 * buffer function
 ************************/
void new_buffer(struct buffer* buf, uint32_t init_size) {
  buf->data = malloc(init_size);
  buf->size = init_size;
  buf->next = 0;
}

void new_buffer_from_data(struct buffer* buf, uint32_t buf_size, void* data) {
  buf->data = data;
  buf->size = buf_size;
  buf->next = 0;
}

void reserve_space_for_buffer(struct buffer* buf, uint32_t additional) {
  while(buf->next + additional > buf->size) {
    buf->data = realloc(buf->data, buf->size * 2);
    buf->size *= 2;
  }
}

void reset_buffer(struct buffer* buf) {
  buf->next = 0;
}

void delete_buffer(struct buffer* buf) {
  free(buf->data);
  buf->data = NULL;
  buf->size = 0;
  buf->next = 0;
}

int read_buffer(int sfd, struct buffer* buf, int bufsize) {
  char rbuf[bufsize];
  int rsize, rsize_sum = 0;
  uint32_t rsize_all;

  rsize = read(sfd, &rsize_all, sizeof(uint32_t));
  rsize_all = ntohl(rsize_all);

  while(rsize_sum < rsize_all) {
    if(rsize_sum + bufsize > rsize_all)
      rsize = rsize_all - rsize_sum;
    else
      rsize = bufsize;
    rsize = read(sfd, rbuf, rsize);
    serialize_data(buf, rbuf, rsize);
    rsize_sum += rsize;
  }
  buf->next = 0;

  // clear buffer
  fd_set rfds;
  struct timeval tv;
  int retval;
  FD_ZERO(&rfds);
  FD_SET(sfd, &rfds);
  while(1) {
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    retval = select(sfd+1, &rfds, NULL, NULL, &tv);
    if(retval > 0 && read(sfd, rbuf, bufsize) <= 0) {
      if(read(sfd, rbuf, bufsize) <= 0) break;
    }
    else break;
  }

  return rsize_all;
}

int write_buffer(int sfd, struct buffer* buf, int bufsize) {
  int wsize, wsize_sum = 0;
  uint32_t wsize_all = buf->next;
  uint32_t wsize_all_n = htonl(wsize_all);
  wsize = write(sfd, &wsize_all_n, sizeof(uint32_t));
  while(wsize_sum < wsize_all) {
    if(wsize_sum + bufsize > wsize_all)
      wsize = buf->next - wsize_sum;
    else
      wsize = bufsize;
    wsize = write(sfd, (char*)buf->data + wsize_sum, wsize);
    wsize_sum += wsize;
  }

  // wait for buffer
  fd_set wfds;
  struct timeval tv;
  int retval;
  FD_ZERO(&wfds);
  FD_SET(sfd, &wfds);
  while(1) {
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    retval = select(sfd+1, NULL, &wfds, NULL, &tv);
    if(retval > 0) break;
    else continue;
  }

  return wsize_all;
}

/**********
 * serialize / deserialize primitives
 **********/
void serialize_short(struct buffer* buf, short x) {
  //printf("serialize short: %d\n", x);
  x = htons(x);
  reserve_space_for_buffer(buf, sizeof(uint16_t));
  memcpy(buf->data + buf->next, &x, sizeof(uint16_t));
  buf->next += sizeof(uint16_t);
}

short deserialize_short(struct buffer* buf) {
  uint16_t x;
  memcpy(&x, buf->data + buf->next, sizeof(uint16_t));
  buf->next += sizeof(uint16_t);
  x = ntohs(x);
  //printf("deserialize short: %d\n", *x);
  return x;
}

void serialize_long(struct buffer* buf, long x) {
  //printf("serialize long: %d\n", x);
  x = htonl(x);
  reserve_space_for_buffer(buf, sizeof(uint32_t));
  memcpy(buf->data + buf->next, &x, sizeof(uint32_t));
  buf->next += sizeof(uint32_t);
}

long deserialize_long(struct buffer* buf) {
  uint32_t x;
  memcpy(&x, buf->data + buf->next, sizeof(uint32_t));
  buf->next += sizeof(uint32_t);
  x = ntohl(x);
  //printf("deserialize long %d\n", *x);
  return x;
}

void serialize_data(struct buffer* buf, void* data, uint32_t size) {
  reserve_space_for_buffer(buf, size);
  memcpy((char*)buf->data + buf->next, data, size);
  buf->next += size;
}

void deserialize_data(void* data, struct buffer* buf, uint32_t size) {
  memcpy(data, (char*)buf->data + buf->next, size);
  buf->next += size;
}

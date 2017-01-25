#pragma once

#include <stdio.h>
#include <stdint.h>
#include "buffer.h"
#include "date.h"

/* a profile */

typedef enum {
  PC_ID = 1, PC_NAME, PC_EST, PC_ADDR, PC_MISC, PC_END
} profile_column;

#define NAME_SIZE 70
#define ADDR_SIZE 70

struct profile {
  uint32_t id;
  char name[NAME_SIZE];
  struct date est;
  char addr[ADDR_SIZE];
  char* misc;
};

struct profile_table {
  int n_entries;
  struct profile* entries;
};

/* profile_table */
struct profile_view {
  int n_entries;
  struct profile** p_entries;
};

typedef enum {
  PS_SUCCESS, PS_INVCMD, PS_INVARG,
  PS_INVID, PS_INVDATE, PS_INVCOL
} profile_status;

/* parse and dump a profile from/to string */
int parse_profile_csv(struct profile* ret, char* str);
int dump_profile_csv(char* ret, struct profile* p);
int dump_profile_fancy(char* ret, struct profile* p);

/* allocate and free(deallocate) profile_table/profile_view */
int delete_profile(struct profile* p);
int new_profile_table(struct profile_table* t, int n_max_profile_table);
int clear_profile_table(struct profile_table* t);
int delete_profile_table(struct profile_table* t);
int new_profile_view(struct profile_view* v, int n_profile_table);
int clear_profile_view(struct profile_view* v);
int delete_profile_view(struct profile_view* v);

/* make profile_view from profile_table */
int create_view_from(struct profile_view* v, struct profile_table* t);
/* transform profile_view */
int copy_view(struct profile_view* t, struct profile_view* s);
int take(struct profile_view* to, struct profile_view* from, int n);
int search_word_in(struct profile_view* to, struct profile_view* from,
                   char* word, profile_column which);
int search_word_in_all(struct profile_view* to, struct profile_view* from,
                       char* word);
int sorted_by(struct profile_view* w, struct profile_view* v,
              profile_column which);

/* read and write profile_table */
int read_profiles_csv(struct profile_table* t, FILE* fp);
int write_profiles_csv(FILE* fp, struct profile_view* v);
int write_profiles_fancy(FILE* fp, struct profile_view* v);

/* serialize / deserialize profile, profile_table, profile_view */
void serialize_profile(struct buffer* buf, struct profile* p);
void serialize_profile_table(struct buffer* buf, struct profile_table* t);
void serialize_profile_view(struct buffer* buf, struct profile_view* v);
void deserialize_profile(struct profile* p, struct buffer* buf);
void deserialize_profile_table(struct profile_table* t, struct buffer* buf);

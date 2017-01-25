
#include "profile.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <unistd.h>

#include "generic_search.h"
#include "generic_sort.h"
#include "mystring.h"

/************************
 * handling single profile
 ************************/
int parse_profile_csv(struct profile* ret, char* str) {
  char *splited[5], *e;
  char buf[strlen(str) + 1];
  int n;
  memcpy(buf, str, sizeof(buf));

  n = split(buf, splited, ',', 5);
  if(n != 5) return 0;
  ret->id = strtol(splited[0], &e, 10);
  if(*e != '\0') return 0;
  strcpy(ret->name, splited[1]);
  if(!parse_date(&(ret->est), splited[2]))
    return 0;
  strcpy(ret->addr, splited[3]);
  ret->misc = (char*) malloc(strlen(splited[4]) + 1);
  strcpy(ret->misc, splited[4]);
  return 1;
}

int dump_profile_csv(char* ret, struct profile* p) {
  char buf[20];
  dump_date_csv(buf, &p->est);
  return sprintf(ret, "%d,%s,%s,%s,%s",
                 p->id, p->name, buf, p->addr, p->misc);
}

int dump_profile_fancy(char* ret, struct profile* p) {
  char buf[20];
  dump_date_fancy(buf, &p->est);
  return sprintf(ret, "ID:\t%d\nName:\t%s\nEst.:\t%s\nAddr.:\t%s\nMisc.:\t%s",
                 p->id, p->name, buf, p->addr, p->misc);

}

/************************
 * handling profile_table
 ************************/
int delete_profile(struct profile* p) {
  free(p->misc);
  p->misc = 0;
  return 1;
}

int new_profile_table(struct profile_table* t, int max_n_profile_table) {
  t->n_entries = 0;
  t->entries = (struct profile*)
    malloc(sizeof(struct profile)*max_n_profile_table);
  return max_n_profile_table;
}

int clear_profile_table(struct profile_table* t) {
  t->n_entries = 0;
  return 1;
}

int delete_profile_table(struct profile_table* t) {
  int i, deleted_entries = t->n_entries;
  for(i = 0; i < t->n_entries; i++)
    delete_profile(&t->entries[i]);
  free(t->entries);
  t->n_entries = 0;
  t->entries = NULL;
  return deleted_entries;
}

int new_profile_view(struct profile_view* v, int n_profile_table) {
  v->n_entries = 0;
  v->p_entries = (struct profile**)
    malloc(sizeof(struct profile*)*n_profile_table);
  return n_profile_table;
}

int clear_profile_view(struct profile_view* v) {
  v->n_entries = 0;
  return 1;
}

int delete_profile_view(struct profile_view* v) {
  int deleted_entries = v->n_entries;
  free(v->p_entries);
  v->n_entries = 0;
  v->p_entries = NULL;
  return deleted_entries;
}

/*
 * convert profile_table to profile_view
 */
int create_view_from(struct profile_view* v, struct profile_table* t) {
  int i;
  v->n_entries = 0;
  for(i = 0; i < t->n_entries; i++) {
    v->p_entries[i] = &t->entries[i];
    v->n_entries++;
  }
  return v->n_entries;
}

/*
 * profile getter and comparator
 */
int get_profile_id(struct profile* p) {
  return p->id;
}

char* get_profile_name(struct profile* p) {
  return p->name;
}

struct date* get_profile_est(struct profile* p) {
  return &p->est;
}

char* get_profile_addr(struct profile* p) {
  return p->addr;
}

char* get_profile_misc(struct profile* p) {
  return p->misc;
}

int comp_int(int a, int b) {
  return b - a;
}

int comp_str(char* a, char* b) {
  return strcmp(b, a);
}

int comp_date(struct date* a, struct date* b) {
  if(b->y != a->y) return b->y - a->y;
  if(b->m != a->m) return b->m - a->m;
  return b->d - a->d;
}

/*
 * define search_profile and sort_profile
 */
generic_search(profile_str, struct profile*, char*);
generic_sort(profile_str, struct profile*, char*);
generic_search(profile_int, struct profile*, int);
generic_sort(profile_int, struct profile*, int);
generic_search(profile_date, struct profile*, struct date*);
generic_sort(profile_date, struct profile*, struct date*);

/*
 * transform profile_view
 */
int copy_view(struct profile_view* t, struct profile_view* s) {
  t->n_entries = s->n_entries;
  memcpy(t->p_entries, s->p_entries, sizeof(struct profile*)*s->n_entries);
  return t->n_entries;
}

int take(struct profile_view* to, struct profile_view* from, int n) {
  int i, j = 0, minidx = 0, maxidx = from->n_entries;
  if(n >= 0 && n <= from->n_entries) maxidx = n;
  else if(n < 0 && n >= -from->n_entries) minidx = from->n_entries + n;
  else return PS_INVARG;

  to->n_entries = maxidx - minidx;
  for(i = minidx; i < maxidx; i++) {
    to->p_entries[j++] = from->p_entries[i];
  }
  return PS_SUCCESS;
}

int search_word_in(struct profile_view* to, struct profile_view* from,
                   char* word, profile_column which) {
  int found_idx[from->n_entries];
  int i, n_found = 0;
  int id;
  char* e;
  struct date est;

  switch(which) {
  case(PC_ID):
    id = strtol(word, &e, 10);
    if(*e == '\0') {
      n_found = search_profile_int(found_idx, from->p_entries, id,
                                   from->n_entries,
                                   get_profile_id, comp_int);
    } else {
      // invalid id
      return PS_INVID;
    }
    break;
  case(PC_NAME):
    n_found = search_profile_str(found_idx, from->p_entries, word,
                                 from->n_entries,
                                 get_profile_name, comp_str);
    break;
  case(PC_EST):
    if(parse_date(&est, word)) {
      n_found = search_profile_date(found_idx, from->p_entries, &est,
                                    from->n_entries,
                                    get_profile_est, comp_date);
    } else {
      // invalid date
      return PS_INVDATE;
    }
    break;
  case(PC_ADDR):
    n_found = search_profile_str(found_idx, from->p_entries, word,
                                 from->n_entries,
                                 get_profile_addr, comp_str);
    break;
  case(PC_MISC):
    n_found = search_profile_str(found_idx, from->p_entries, word,
                                 from->n_entries,
                                 get_profile_misc, comp_str);
    break;
  default:
    // invalid column number
    return PS_INVCOL;
    break;
  }

  to->n_entries = n_found;
  for(i = 0; i < n_found; i++)
    to->p_entries[i] = from->p_entries[found_idx[i]];
  return PS_SUCCESS;
}

int search_word_in_all(struct profile_view* to, struct profile_view* from,
                       char* word) {
  int idx[from->n_entries];
  struct profile_view result_per_col;
  new_profile_view(&result_per_col, from->n_entries);

  int i, j, col, n_found = 0;
  for(i = 0; i < from->n_entries; i++)
    idx[i] = 0;
  for(col = PC_ID; col < PC_END; col++) {
    if(search_word_in(&result_per_col, from, word, col) != PS_SUCCESS)
      continue;
    // TODO poor performance (algorithm)
    for(i = 0; i < from->n_entries; i++)
      for(j = 0; j < result_per_col.n_entries; j++)
        if(idx[i] == 0 &&
           from->p_entries[i] == result_per_col.p_entries[j]) {
          idx[i] = 1;
          n_found++;
        }
  }
  delete_profile_view(&result_per_col);

  i = 0;
  to->n_entries = n_found;
  for(j = 0; j < n_found; j++) {
    while(idx[i] == 0) i++;
    to->p_entries[j] = from->p_entries[i++];
  }
  return PS_SUCCESS;
}

int sorted_by(struct profile_view* w,  struct profile_view* v,
              profile_column which) {
  copy_view(w, v);
  switch(which) {
  case(PC_ID):
    sort_profile_int(w->p_entries, w->n_entries,
                     get_profile_id, comp_int);
    break;
  case(PC_NAME):
    sort_profile_str(w->p_entries, w->n_entries,
                     get_profile_name, comp_str);
    break;
  case(PC_EST):
    sort_profile_date(w->p_entries, w->n_entries,
                      get_profile_est, comp_date);
    break;
  case(PC_ADDR):
    sort_profile_str(w->p_entries, w->n_entries,
                     get_profile_addr, comp_str);
    break;
  case(PC_MISC):
    sort_profile_str(w->p_entries, w->n_entries,
                     get_profile_misc, comp_str);
    break;
  default:
    // invalid column number
    return PS_INVCOL;
  }

  return PS_SUCCESS;
}

/*
 * read / write function
 */
int read_profiles_csv(struct profile_table* t, FILE* fp) {
  int i, n_added = 0;
  char buf[1024];

  while(fgets(buf, 1024, fp)) {
    // stlip newline
    for(i = 0; i < 1024 && buf[i] != '\0'; i++)
      if(buf[i] == '\r' || buf[i] == '\n') buf[i] = '\0';
    if(parse_profile_csv(&t->entries[t->n_entries], buf)) {
      t->n_entries++;
      n_added++;
    }
  }
  return n_added;
}

int write_profiles_csv(FILE* fp, struct profile_view* v) {
  int i;
  char buf[1024];
  for(i = 0; i < v->n_entries; i++) {
    dump_profile_csv(buf, v->p_entries[i]);
    fprintf(fp, "%s", buf);
    //if(i < v->n_entries-1)
      fprintf(fp, "\n");
  }
  return i;
}

int write_profiles_fancy(FILE* fp, struct profile_view* v) {
  int i;
  char buf[1024];
  for(i = 0; i < v->n_entries; i++) {
    dump_profile_fancy(buf, v->p_entries[i]);
    fprintf(fp, "%s\n\n", buf);
  }
  return i;
}

void serialize_profile(struct buffer* buf, struct profile* p) {
  serialize_long(buf, p->id);
  serialize_data(buf, p->name, NAME_SIZE);
  serialize_date(buf, &p->est);
  serialize_data(buf, p->addr, ADDR_SIZE);
  serialize_str(buf, p->misc);
}

void serialize_profile_table(struct buffer* buf, struct profile_table* t) {
  int i;
  serialize_long(buf, t->n_entries);
  for(i = 0; i < t->n_entries; i++)
    serialize_profile(buf, &t->entries[i]);
}

void serialize_profile_view(struct buffer* buf, struct profile_view* v) {
  int i;
  serialize_long(buf, v->n_entries);
  for(i = 0; i < v->n_entries; i++)
    serialize_profile(buf, v->p_entries[i]);
}

void deserialize_profile(struct profile* p, struct buffer* buf) {
  p->id = deserialize_long(buf);
  deserialize_data(p->name, buf, NAME_SIZE);
  deserialize_date(&p->est, buf);
  deserialize_data(p->addr, buf, ADDR_SIZE);
  deserialize_str(&p->misc, buf);
}

void deserialize_profile_table(struct profile_table* t, struct buffer* buf) {
  int i;
  t->n_entries = deserialize_long(buf);
  for(i = 0; i < t->n_entries; i++)
    deserialize_profile(&t->entries[i], buf);
}

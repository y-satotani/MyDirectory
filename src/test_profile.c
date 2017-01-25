
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "profile.h"

int main(int argc, char* argv[]) {
  // single profile test
  char* csv = "5100046,The Bridge,1845-11-2,14 Seafield Road Longman Inverness,SEN Unit 2.0 Open";
  struct profile p;
  assert(parse_profile_csv(&p, csv));
  assert(p.id == 5100046);
  assert(strcmp(p.name, "The Bridge") == 0);
  assert(p.est.y == 1845 && p.est.m == 11 && p.est.d == 2);
  assert(strcmp(p.addr, "14 Seafield Road Longman Inverness") == 0);
  assert(strcmp(p.misc, "SEN Unit 2.0 Open") == 0);

  // table and view test
  struct profile_table t;
  struct profile_view v, w;
  new_profile_table(&t, 10000);
  new_profile_view(&v, 10000);
  new_profile_view(&w, 10000);
  FILE* fp = fopen("sample.csv", "r");
  assert(read_profiles_csv(&t, fp) == 2886);
  fclose(fp);
  assert(create_view_from(&v, &t) == 2886);
  assert(take(&w, &v, 10) == PS_SUCCESS);
  assert(w.n_entries == 10);
  assert(take(&w, &v, -1) == PS_SUCCESS);
  assert(w.n_entries == 1);
  assert(search_word_in_all(&w, &v, "The Bridge") == PS_SUCCESS);
  assert(w.n_entries == 1);
  assert(search_word_in_all(&w, &v, "5100046") == PS_SUCCESS);
  assert(w.n_entries == 1);
  assert(sorted_by(&w, &v, PC_ID) == PS_SUCCESS);
  assert(w.p_entries[0]->id == 5100046);
  //write_profiles_fancy(stdout, &w);
  //write_profiles_csv(stdout, &w);
  delete_profile_view(&v);
  delete_profile_view(&w);
  delete_profile_table(&t);
}

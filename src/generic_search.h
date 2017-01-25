#pragma once

#define generic_search(suffix, t1, t2)          \
  int search_##suffix(int idx[],                \
                      t1 arr[],                 \
                      t2 obj,                   \
                      int n,                    \
                      t2(*key)(t1),             \
                      int(*comp)(t2, t2)) {     \
    int i;                                      \
    int n_found = 0;                            \
    for(i = 0; i < n; i++) {                    \
      if(comp(key(arr[i]), obj) == 0)           \
        idx[n_found++] = i;                     \
    }                                           \
    return n_found;                             \
  }

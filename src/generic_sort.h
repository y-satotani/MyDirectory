#pragma once

#define generic_sort(suffix, t1, t2)            \
  void sort_##suffix(t1 arr[],                  \
                     int n,                     \
                     t2(*key)(t1),              \
                     int(*comp)(t2, t2)) {      \
    int i, j;                                   \
    t1 t_a;                                     \
    t2 keys[n];                                 \
    t2 t_k;                                     \
                                                \
    for(i = 0; i < n; i++) {                    \
      keys[i] = key(arr[i]);                    \
    }                                           \
                                                \
    for(j = 0; j < n-1; j++) {                  \
      for(i = 0; i < n-j-1; i++) {              \
        if(comp(keys[i], keys[i+1]) < 0) {      \
          t_a = arr[i];                         \
          arr[i] = arr[i+1];                    \
          arr[i+1] = t_a;                       \
                                                \
          t_k = keys[i];                        \
          keys[i] = keys[i+1];                  \
          keys[i+1] = t_k;                      \
        }                                       \
      }                                         \
    }                                           \
                                                \
  }

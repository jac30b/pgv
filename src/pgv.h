#ifndef PGV_H
#define PGV_H

#include "c.h"

#define MAX_VEC_DIM 1024

int32 vec_sz(int32 dim);

typedef struct Vec {
  int32 vl_len_; // varlena header
  int32 dim;
  float data[FLEXIBLE_ARRAY_MEMBER];
} Vec;

#endif

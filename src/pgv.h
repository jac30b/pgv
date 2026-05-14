#ifndef PGV_H
#define PGV_H

#include "c.h"
typedef struct Vec {
  int32 len;
  int16 dim;
  float data[FLEXIBLE_ARRAY_MEMBER];
} Vec;

#endif

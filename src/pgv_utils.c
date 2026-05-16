#include "pgv_utils.h"
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int32 vec_sz(int32 dim) { return offsetof(Vec, data) + dim * sizeof(float); }

float vec_cosine_similarity_internal(Vec *a, Vec *b) {
  float dot = 0.0f;
  float norm_a = 0.0f;
  float norm_b = 0.0f;
  int i;

  for (i = 0; i < a->dim; i++) {
    dot += a->data[i] * b->data[i];
    norm_a += a->data[i] * a->data[i];
    norm_b += b->data[i] * b->data[i];
  }

  norm_a = sqrtf(norm_a);
  norm_b = sqrtf(norm_b);

  if (norm_a == 0.0f || norm_b == 0.0f)
    return 0.0f;

  return dot / (norm_a * norm_b);
}

int vec_parse(const char *str, float *buf, int max_dim, const char **errmsg) {
  const char *ptr = str;
  int dim = 0;

  while (isspace((unsigned char)*ptr))
    ptr++;

  if (*ptr != '[') {
    *errmsg = "invalid input syntax for vec: expected '['";
    return -1;
  }
  ptr++;

  while (*ptr) {
    float val;
    char *endptr;

    if (dim >= max_dim) {
      *errmsg = "vector exceeds maximum dimension";
      return -1;
    }

    while (isspace((unsigned char)*ptr))
      ptr++;

    if (*ptr == ']')
      break;

    val = strtof(ptr, &endptr);

    if (ptr == endptr) {
      *errmsg = "invalid input syntax for vec: expected float";
      return -1;
    }

    buf[dim++] = val;
    ptr = endptr;

    while (isspace((unsigned char)*ptr))
      ptr++;

    if (*ptr == ',') {
      ptr++;
    } else if (*ptr == ']') {
      break;
    } else {
      *errmsg = "invalid input syntax for vec: expected ',' or ']'";
      return -1;
    }
  }

  if (*ptr != ']') {
    *errmsg = "invalid input syntax for vec: expected ']'";
    return -1;
  }

  if (dim == 0) {
    *errmsg = "invalid input syntax for vec: empty vector";
    return -1;
  }

  return dim;
}

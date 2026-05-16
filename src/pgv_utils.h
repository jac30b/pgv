#ifndef PGV_UTILS_H
#define PGV_UTILS_H

#include "pgv.h"

float vec_cosine_similarity_internal(Vec *a, Vec *b);
int vec_parse(const char *str, float *buf, int max_dim, const char **errmsg);

#endif

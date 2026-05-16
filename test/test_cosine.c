#include <criterion/criterion.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "pgv_utils.h"

static Vec *make_vec(float *data, int dim) {
  Vec *v = malloc(vec_sz(dim));
  v->vl_len_ = 0;
  v->dim = dim;
  for (int i = 0; i < dim; i++) {
    v->data[i] = data[i];
  }
  return v;
}

Test(cosine, identical_vectors) {
  float data_a[] = {1.0f, 2.0f, 3.0f};
  float data_b[] = {1.0f, 2.0f, 3.0f};
  Vec *a = make_vec(data_a, 3);
  Vec *b = make_vec(data_b, 3);
  float sim = vec_cosine_similarity_internal(a, b);
  cr_assert_float_eq(sim, 1.0f, 1e-6f);
  free(a);
  free(b);
}

Test(cosine, orthogonal_vectors) {
  float data_a[] = {1.0f, 0.0f, 0.0f};
  float data_b[] = {0.0f, 1.0f, 0.0f};
  Vec *a = make_vec(data_a, 3);
  Vec *b = make_vec(data_b, 3);
  float sim = vec_cosine_similarity_internal(a, b);
  cr_assert_float_eq(sim, 0.0f, 1e-6f);
  free(a);
  free(b);
}

Test(cosine, opposite_vectors) {
  float data_a[] = {1.0f, 2.0f, 3.0f};
  float data_b[] = {-1.0f, -2.0f, -3.0f};
  Vec *a = make_vec(data_a, 3);
  Vec *b = make_vec(data_b, 3);
  float sim = vec_cosine_similarity_internal(a, b);
  cr_assert_float_eq(sim, -1.0f, 1e-6f);
  free(a);
  free(b);
}

Test(cosine, zero_vector) {
  float data_a[] = {0.0f, 0.0f, 0.0f};
  float data_b[] = {1.0f, 2.0f, 3.0f};
  Vec *a = make_vec(data_a, 3);
  Vec *b = make_vec(data_b, 3);
  float sim = vec_cosine_similarity_internal(a, b);
  cr_assert_float_eq(sim, 0.0f, 1e-6f);
  free(a);
  free(b);
}

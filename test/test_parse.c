#include <criterion/criterion.h>
#include <string.h>

#include "pgv_utils.h"

Test(parse, basic_vector) {
  float buf[1024];
  const char *err = NULL;
  int dim = vec_parse("[1,2,3]", buf, 1024, &err);
  cr_assert_eq(dim, 3);
  cr_assert_float_eq(buf[0], 1.0f, 1e-6f);
  cr_assert_float_eq(buf[1], 2.0f, 1e-6f);
  cr_assert_float_eq(buf[2], 3.0f, 1e-6f);
}

Test(parse, with_spaces) {
  float buf[1024];
  const char *err = NULL;
  int dim = vec_parse("[ 1 , 2 , 3 ]", buf, 1024, &err);
  cr_assert_eq(dim, 3);
  cr_assert_float_eq(buf[0], 1.0f, 1e-6f);
  cr_assert_float_eq(buf[1], 2.0f, 1e-6f);
  cr_assert_float_eq(buf[2], 3.0f, 1e-6f);
}

Test(parse, empty_vector) {
  float buf[1024];
  const char *err = NULL;
  int dim = vec_parse("[]", buf, 1024, &err);
  cr_assert_eq(dim, -1);
  cr_assert_not_null(err);
  cr_assert_str_eq(err, "invalid input syntax for vec: empty vector");
}

Test(parse, missing_open_bracket) {
  float buf[1024];
  const char *err = NULL;
  int dim = vec_parse("1,2,3]", buf, 1024, &err);
  cr_assert_eq(dim, -1);
  cr_assert_not_null(err);
}

Test(parse, missing_close_bracket) {
  float buf[1024];
  const char *err = NULL;
  int dim = vec_parse("[1,2,3", buf, 1024, &err);
  cr_assert_eq(dim, -1);
  cr_assert_not_null(err);
}

Test(parse, invalid_token) {
  float buf[1024];
  const char *err = NULL;
  int dim = vec_parse("[1;2;3]", buf, 1024, &err);
  cr_assert_eq(dim, -1);
  cr_assert_not_null(err);
}

Test(parse, max_dim_exceeded) {
  float buf[2];
  const char *err = NULL;
  int dim = vec_parse("[1,2,3]", buf, 2, &err);
  cr_assert_eq(dim, -1);
  cr_assert_not_null(err);
}

Test(parse, floats) {
  float buf[1024];
  const char *err = NULL;
  int dim = vec_parse("[1.5,-2.5,3.0e2]", buf, 1024, &err);
  cr_assert_eq(dim, 3);
  cr_assert_float_eq(buf[0], 1.5f, 1e-6f);
  cr_assert_float_eq(buf[1], -2.5f, 1e-6f);
  cr_assert_float_eq(buf[2], 300.0f, 1e-6f);
}

Test(parse, single_element) {
  float buf[1024];
  const char *err = NULL;
  int dim = vec_parse("[42]", buf, 1024, &err);
  cr_assert_eq(dim, 1);
  cr_assert_float_eq(buf[0], 42.0f, 1e-6f);
}

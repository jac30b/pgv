#include "postgres.h"

#include <math.h>

#include "utils/array.h"
#include "pgv.h"
#include "pgv_utils.h"

#include "fmgr.h"
#include "utils/builtins.h"
#include "libpq/pqformat.h"
#include "port.h"

PG_MODULE_MAGIC;

// (cstring, oid, integer)
PGDLLEXPORT PG_FUNCTION_INFO_V1(vec_input);
Datum vec_input(PG_FUNCTION_ARGS) {
  float buf[MAX_VEC_DIM];
  const char *err;
  Vec *res;

  const char *str = PG_GETARG_CSTRING(0);
  const int dim = vec_parse(str, buf, MAX_VEC_DIM, &err);
  if (dim < 0)
    ereport(ERROR, (errmsg("%s", err)));

  res = palloc0(vec_sz(dim));
  res->dim = dim;
  for (int i = 0; i < dim; i++) {
    res->data[i] = buf[i];
  }
  SET_VARSIZE(res, vec_sz(dim));
  PG_RETURN_POINTER(res);
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(vec_output);
Datum vec_output(PG_FUNCTION_ARGS) {
  const Vec *vec = (Vec *)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  StringInfoData buf;
  int i;

  initStringInfo(&buf);
  appendStringInfo(&buf, "<%d>[", vec->dim);
  for (i = 0; i < vec->dim; i++) {
    char *f;
    if (i > 0)
      appendStringInfoChar(&buf, ',');
    f = DatumGetCString(
        DirectFunctionCall1(float4out, Float4GetDatum(vec->data[i])));
    appendStringInfoString(&buf, f);
    pfree(f);
  }
  appendStringInfoChar(&buf, ']');

  PG_RETURN_CSTRING(buf.data);
}

// cstring[]
PGDLLEXPORT PG_FUNCTION_INFO_V1(vec_typemodifier_in);
Datum vec_typemodifier_in(PG_FUNCTION_ARGS) {
  ArrayType *arr = PG_GETARG_ARRAYTYPE_P(0);
  int32 *dim;
  int ndim;

  dim = ArrayGetIntegerTypmods(arr, &ndim);
  elog(INFO, "ndim: %d", ndim);

  if (ndim != 1) {
    ereport(ERROR, (errmsg("vec type must have exactly one type modifier")));
  }

  elog(INFO, "dim[0]: %d", dim[0]);

  if (dim[0] <= 0) {
    ereport(ERROR, (errmsg("vec type dimension must be positive")));
  }

  PG_RETURN_INT32(dim[0]);
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(vec_typemodifier_out);
Datum vec_typemodifier_out(PG_FUNCTION_ARGS) {
  char *buf;
  const int32 typemod = PG_GETARG_INT32(0);

  if (typemod < 0) {
    PG_RETURN_CSTRING(pstrdup(""));
  }

  buf = psprintf("%d", typemod);
  PG_RETURN_CSTRING(buf);
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(vec_cosine_distance);
Datum vec_cosine_distance(PG_FUNCTION_ARGS) {
  Vec *a = (Vec *)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  Vec *b = (Vec *)PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
  float similarity;
  float distance;

  if (a->dim != b->dim)
    ereport(
        ERROR,
        (errmsg("vectors must have the same dimension for cosine distance")));

  similarity = vec_cosine_similarity_internal(a, b);
  distance = 1.0f - similarity;

  /* Clamp to [0, 1] to avoid floating point drift */
  if (distance < 0.0f)
    distance = 0.0f;
  else if (distance > 1.0f)
    distance = 1.0f;

  PG_RETURN_FLOAT4(distance);
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(vec_cmp);
Datum vec_cmp(PG_FUNCTION_ARGS) {
  Vec *a = (Vec *)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  Vec *b = (Vec *)PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

  PG_RETURN_INT32(vec_cmp_internal(a, b));
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(vec_lt);
Datum vec_lt(PG_FUNCTION_ARGS) {
  Vec *a = (Vec *)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  Vec *b = (Vec *)PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

  PG_RETURN_BOOL(vec_cmp_internal(a, b) < 0);
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(vec_le);
Datum vec_le(PG_FUNCTION_ARGS) {
  Vec *a = (Vec *)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  Vec *b = (Vec *)PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

  PG_RETURN_BOOL(vec_cmp_internal(a, b) <= 0);
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(vec_eq);
Datum vec_eq(PG_FUNCTION_ARGS) {
  Vec *a = (Vec *)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  Vec *b = (Vec *)PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

  PG_RETURN_BOOL(vec_cmp_internal(a, b) == 0);
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(vec_ne);
Datum vec_ne(PG_FUNCTION_ARGS) {
  Vec *a = (Vec *)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  Vec *b = (Vec *)PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

  PG_RETURN_BOOL(vec_cmp_internal(a, b) != 0);
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(vec_ge);
Datum vec_ge(PG_FUNCTION_ARGS) {
  Vec *a = (Vec *)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  Vec *b = (Vec *)PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

  PG_RETURN_BOOL(vec_cmp_internal(a, b) >= 0);
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(vec_gt);
Datum vec_gt(PG_FUNCTION_ARGS) {
  Vec *a = (Vec *)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  Vec *b = (Vec *)PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

  PG_RETURN_BOOL(vec_cmp_internal(a, b) > 0);
}

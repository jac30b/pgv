#include "postgres.h"

#include "utils/array.h"
#include "pgv.h"

#include "fmgr.h"

PG_MODULE_MAGIC;


int32 vec_sz(int32 dim) { return offsetof(Vec, data) + dim * sizeof(float); }

// (cstring, oid, integer)
PGDLLEXPORT PG_FUNCTION_INFO_V1(vec_input);
Datum vec_input(PG_FUNCTION_ARGS) {
  Vec *res;
  char *str;
  Oid typelem;
  int32 typemod;
  int sz;

  str = PG_GETARG_CSTRING(0);
  typelem = PG_GETARG_OID(1);
  typemod = PG_GETARG_INT32(2);

  elog(INFO, "input string: %s", str);
  elog(INFO, "type element OID: %u", typelem);
  elog(INFO, "type modifier: %d", typemod);
  elog(INFO, "nargs: %d", PG_NARGS());

  if (typemod == -1) {
    elog(WARNING, "typemod is -1, checking type info...");
  }

  sz = vec_sz(typemod);
  res = (Vec *)palloc(sz);
  SET_VARSIZE(res, sz);
  res->dim = typemod;

  PG_RETURN_POINTER(res);
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(vec_output);
Datum vec_output(PG_FUNCTION_ARGS) {
  Vec* vec = (Vec*)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

  char* buf = psprintf("<%d>", vec->dim);
  PG_RETURN_CSTRING(buf);
}


// cstring[]
PGDLLEXPORT PG_FUNCTION_INFO_V1(vec_typemodifier_in);
Datum vec_typemodifier_in(PG_FUNCTION_ARGS) {
  ArrayType *arr = PG_GETARG_ARRAYTYPE_P(0);
  int32 *dim;
  int ndim;

  elog(INFO, "vec_typemodifier_in called");
  elog(INFO, "array is null: %d", arr == NULL);
  
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
  int32 typemod = PG_GETARG_INT32(0);
  char *buf;

  if (typemod < 0) {
    PG_RETURN_CSTRING(pstrdup(""));
  }

  buf = psprintf("%d", typemod);
  PG_RETURN_CSTRING(buf);
}

// PG_FUNCTION_INFO_V1(vector_out);
// Datum vec_output(PG_FUNCTION_ARGS) {
//   Vec* vec = (Vec*)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
//   char* buf;

//   PG_RETURN_CSTRING(buf);
// }

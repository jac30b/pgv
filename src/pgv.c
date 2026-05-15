#include "pgv.h"

#include "fmgr.h"
#include "postgres.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(add_nums);
Datum add_nums(PG_FUNCTION_ARGS) {
  int32 arg1 = PG_GETARG_INT32(0);
  int32 arg2 = PG_GETARG_INT32(1);

  PG_RETURN_INT32(arg1 + arg2);
}

PG_FUNCTION_INFO_V1(vec_input);
Datum vec_input(PG_FUNCTION_ARGS) {
  Vec* res;

  char* str = PG_GETARG_CSTRING(0);
  elog(INFO, "input string: %s", str);

  PG_RETURN_POINTER(res);
}

PG_FUNCTION_INFO_V1(vector_out);
Datum vec_output(PG_FUNCTION_ARGS) {
  Vec* vec = (Vec*)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
  char* buf;

  PG_RETURN_CSTRING(buf);
}

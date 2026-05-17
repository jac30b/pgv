CREATE TYPE vec;

CREATE FUNCTION vec_input(cstring, oid, integer)
RETURNS vec AS 'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION vec_output(vec)
RETURNS cstring AS 'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION vec_typemodifier_in(cstring[])
RETURNS integer AS 'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION vec_typemodifier_out(integer)
RETURNS cstring AS 'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION vec_cosine_distance(vec, vec)
RETURNS float4 AS 'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION vec_cmp(vec, vec)
RETURNS integer AS 'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION vec_lt(vec, vec)
RETURNS boolean AS 'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION vec_le(vec, vec)
RETURNS boolean AS 'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION vec_eq(vec, vec)
RETURNS boolean AS 'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION vec_ne(vec, vec)
RETURNS boolean AS 'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION vec_ge(vec, vec)
RETURNS boolean AS 'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION vec_gt(vec, vec)
RETURNS boolean AS 'MODULE_PATHNAME' LANGUAGE C IMMUTABLE STRICT;

CREATE OPERATOR < (
    LEFTARG = vec,
    RIGHTARG = vec,
    PROCEDURE = vec_lt,
    COMMUTATOR = >,
    NEGATOR = >=
);

CREATE OPERATOR <= (
    LEFTARG = vec,
    RIGHTARG = vec,
    PROCEDURE = vec_le,
    COMMUTATOR = >=,
    NEGATOR = >
);

CREATE OPERATOR = (
    LEFTARG = vec,
    RIGHTARG = vec,
    PROCEDURE = vec_eq,
    COMMUTATOR = =,
    NEGATOR = <>
);

CREATE OPERATOR <> (
    LEFTARG = vec,
    RIGHTARG = vec,
    PROCEDURE = vec_ne,
    COMMUTATOR = <>,
    NEGATOR = =
);

CREATE OPERATOR >= (
    LEFTARG = vec,
    RIGHTARG = vec,
    PROCEDURE = vec_ge,
    COMMUTATOR = <=,
    NEGATOR = <
);

CREATE OPERATOR > (
    LEFTARG = vec,
    RIGHTARG = vec,
    PROCEDURE = vec_gt,
    COMMUTATOR = <,
    NEGATOR = <=
);

--  https://www.postgresql.org/docs/current/sql-createtype.html
CREATE TYPE vec (
    INPUT = vec_input,
    OUTPUT = vec_output,
    TYPMOD_IN = vec_typemodifier_in,
    TYPMOD_OUT = vec_typemodifier_out
);

CREATE OPERATOR <-> (
    LEFTARG = vec,
    RIGHTARG = vec,
    PROCEDURE = vec_cosine_distance,
    COMMUTATOR = <->
);

CREATE OPERATOR CLASS vec_ops
    DEFAULT FOR TYPE vec USING btree AS
    OPERATOR 1 <,
    OPERATOR 2 <=,
    OPERATOR 3 =,
    OPERATOR 4 <>,
    OPERATOR 5 >=,
    OPERATOR 6 >,
    FUNCTION 1 vec_cmp(vec, vec);
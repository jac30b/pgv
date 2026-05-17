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
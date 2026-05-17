# PostgreSQL Custom Type Internals — Notes

## 1. Type definition vs value I/O functions

When creating a custom type:

```sql
CREATE TYPE mytype (
  INPUT = mytype_in,
  OUTPUT = mytype_out,
  TYPMOD_IN = mytype_typmod_in
);
```

There are two distinct layers:

### Value functions

* `INPUT / OUTPUT / RECEIVE / SEND`
* Handle **actual values**
* Example: parsing `'[1,2,3]'` into a vector

### Type modifier functions

* `TYPMOD_IN / TYPMOD_OUT`
* Handle **type declaration modifiers**
* Example: `vector(5)` → extract `5`

---

## 2. What handles `vector(5)`?

`vector(5)` is processed by:

### Step 1: typmod parsing

```c
typmod_in(cstring[])
```

* Receives `"5"` as string array
* Converts to `int32` typmod

Example:

```c
Datum vector_typmod_in(PG_FUNCTION_ARGS);
```

Returns:

```text
5
```

---

### Step 2: storage

The result is stored in:

* `pg_attribute.atttypmod`

---

### Step 3: passed into input function

PostgreSQL automatically calls:

```c
vector_in(cstring, oid, int32 typmod)
```

So:

```sql
vector(5)
```

becomes:

```c
vector_in("...", type_oid, 5)
```

---

## 3. Internal typmod helpers

### ArrayGetIntegerTypmods()

* Internal PostgreSQL function
* Parses `cstring[] → int32[]`

Example usage in built-in types:

```c
ArrayGetIntegerTypmods(...)
```

Used by:

* `numeric`
* `interval`
* other multi-parameter types

⚠ Not part of stable public extension API.

---

## 4. Memory allocation in PostgreSQL

### `palloc(size)`

* Allocates memory
* **Uninitialized**

### `palloc0(size)`

* Allocates memory
* **Zero-initialized**

Use:

* `palloc` → performance / overwrite immediately
* `palloc0` → safer default for structs

---

## 5. Variable-length (varlena) system

Most PostgreSQL types are varlena:

```
[length][data...]
```

### Key macros

#### SET_VARSIZE(ptr, len)

Sets total byte size of a varlena object.

```c
SET_VARSIZE(v, total_size);
```

Meaning:

> “This object is `len` bytes long”

### Related macros

* `VARSIZE(x)` → read size
* `VARDATA(x)` → pointer to payload
* `VARHDRSZ` → header size

---


## 6. Logging in PostgreSQL extensions

### Recommended API: `ereport()`

```c
ereport(ERROR,
        errmsg("invalid dimension"));
```

### Simple logging: `elog()`

```c
elog(INFO, "debug message");
```

Levels:

* DEBUG1–5
* INFO
* NOTICE
* WARNING
* ERROR

---

## 7. Shared library export macros

### PGDLLEXPORT

Marks a function as visible to PostgreSQL when loaded dynamically:

```c
PGDLLEXPORT Datum vector_in(PG_FUNCTION_ARGS);
```

Expands to platform-specific visibility control:

* Windows: `__declspec(dllexport)`
* GCC/Clang: visibility attributes

Purpose:

> Ensure PostgreSQL can resolve symbols via dynamic loading.

---

## 8. Key mental model

### PostgreSQL type system has 3 layers:

1. **Type modifiers**

   * `vector(5)`
   * handled by `TYPMOD_IN`

2. **Value parsing**

   * `'[1,2,3]'`
   * handled by `INPUT`

3. **Execution-time enforcement**

   * typmod passed into `INPUT`
   * validation happens in C code

---

## 9. Core insight

* `typmod_in` → parses type declaration (`vector(5)`)
* `INPUT` → parses values
* PostgreSQL injects typmod automatically into input functions
* varlena macros define how data is physically stored
* memory + logging APIs are PostgreSQL-specific (`palloc`, `ereport`)


Here’s the updated `notes.md` entry you can add:

---

## 10. TOAST access: `PG_DETOAST_DATUM`

### What it does

```c id="t8a1k2"
PG_DETOAST_DATUM(datum)
```

Ensures a PostgreSQL value is fully accessible in memory.

If the value is:

* stored inline → returns it directly
* TOASTed (compressed or external) → fetches + decompresses it first

---

### Why it exists

PostgreSQL uses **TOAST (The Oversized-Attribute Storage Technique)** for large values like:

* `text`
* `bytea`
* `jsonb`
* large arrays
* custom varlena types

So a `Datum` may not contain the actual data directly.

---

### What it returns

* A **fully expanded (detoasted) value**
* Safe to read via `VARDATA()` / struct casts

---

### Typical usage

```c id="p3q9m1"
Datum detoasted = PG_DETOAST_DATUM(input);

char *data = VARDATA_ANY(detoasted);
int len = VARSIZE_ANY_EXHDR(detoasted);
```

---

### Variants

#### `PG_DETOAST_DATUM(datum)`

* detoasts only if needed
* may return original or a temporary expanded value

#### `PG_DETOAST_DATUM_COPY(datum)`

* always returns a **new palloc’d copy**
* safe to keep ownership

#### `PG_DETOAST_DATUM_SLICE(datum, offset, len)`

* reads only part of a large value
* avoids full detoast (more efficient)

---

### When to use it

Use when:

* accessing raw `Datum` contents in C
* implementing custom types/functions
* working with potentially large varlena values

---

### When NOT needed

Often unnecessary if you already use:

* `text_to_cstring()`
* higher-level PostgreSQL APIs that detoast internally

---

### Key idea

PostgreSQL values may be stored:

```text
inline OR compressed OR external (TOAST)
```

`PG_DETOAST_DATUM()` guarantees:

> “Give me the real in-memory value, fully expanded and safe to read.”

---


## 12. Type modifiers (`typmod`) + input function caveat

### What is typmod?

A **typmod (type modifier)** is extra per-column metadata that parameterizes a type.

Examples:

```sql id="t1"
vector(5)
varchar(100)
numeric(10,2)
```

It lets one type behave differently per column without creating new types.

---

### What typmod actually is

* Stored as a single `int32`
* Stored in `pg_attribute.atttypmod`
* `-1` means “no typmod”

---

### Typmod flow (ideal model)

```text id="flow1"
vector(5)
  ↓
TYPMOD_IN("5") → 5
  ↓
stored in pg_attribute
  ↓
vec_input(..., typmod=5)
```

---

### Important reality check

Even if `TYPMOD_IN` is defined correctly:

> `vec_input()` may still receive `typmod = -1`

---

### Why typmod becomes -1

Typmod is only passed when:

* value is created via table attribute coercion path
* executor uses `heap_form_tuple` / slot-based insertion

Typmod is NOT passed when:

* calling input function directly
* using generic Datum/function invocation
* bypassing attribute context

---

### Key consequence

> You cannot reliably use typmod inside INPUT functions for enforcing rules like vector dimension.

---

### What typmod IS still useful for

* Column metadata (`pg_attribute.atttypmod`)
* Planner / executor decisions
* Built-in types (`varchar`, `numeric`, `interval`)
* Index/operator logic in some cases

---

### Recommended pattern for custom vector types

Do **not rely on typmod at runtime**.

Instead:

* enforce dimension while parsing the value
* store dimension inside the value (`Vec->dim`)
* validate directly in `INPUT` logic

---

### Key takeaway

> Typmod is column-level metadata for the database engine, not a guaranteed runtime argument for all input function paths.

---

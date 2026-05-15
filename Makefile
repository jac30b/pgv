EXTENSION = pgv
EXTVERSION = 0.0.1

MODULE_big = pgv
DATA = $(wildcard sql/*--*--*.sql)
DATA_built = sql/$(EXTENSION)--$(EXTVERSION).sql
OBJS = src/pgv.o

OPTFLAGS = -march=native

# Mac ARM doesn't always support -march=native
ifeq ($(shell uname -s), Darwin)
	ifeq ($(shell uname -p), arm)
		OPTFLAGS =
	endif
endif

PG_CFLAGS += $(OPTFLAGS) -ftree-vectorize -fassociative-math -fno-signed-zeros -fno-trapping-math

all: sql/$(EXTENSION)--$(EXTVERSION).sql

sql/$(EXTENSION)--$(EXTVERSION).sql: sql/$(EXTENSION).sql
	cp $< $@

PG_CONFIG ?= $(firstword $(shell command -v pg_config 2>/dev/null) $(wildcard /usr/pgsql-*/bin/pg_config) $(wildcard /usr/bin/pg_config))
ifeq ($(shell uname -s), Darwin)
	ifeq ($(shell uname -p), arm)
		PG_CONFIG = /opt/homebrew/opt/postgresql@17/bin/pg_config
	endif
endif
ifeq ($(strip $(PG_CONFIG)),)
$(error could not find pg_config; install PostgreSQL development packages or run 'make install PG_CONFIG=/path/to/pg_config')
endif
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)



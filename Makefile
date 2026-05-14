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

PG_CONFIG ?= pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)



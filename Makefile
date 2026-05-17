EXTENSION = pgv
EXTVERSION = 0.0.1

MODULE_big = pgv
DATA = $(wildcard sql/*--*--*.sql)
DATA_built = sql/$(EXTENSION)--$(EXTVERSION).sql
OBJS = src/pgv.o src/pgv_utils.o src/hnsw.o

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

TEST_DIR = test
TEST_BIN = test_runner
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
PG_INCLUDEDIR_SERVER = $(shell $(PG_CONFIG) --includedir-server)

TEST_CFLAGS = -Wall -Wextra -g -I./src -I$(PG_INCLUDEDIR_SERVER)
TEST_LIBS = -lcriterion -lm

# Run all tests or a specific file: make test_internal FILE=test_cosine.c
.PHONY: test_internal
test_internal:
ifeq ($(FILE),)
	$(CC) $(TEST_SRCS) src/pgv_utils.c $(TEST_CFLAGS) $(TEST_LIBS) -o $(TEST_BIN)
else
	$(CC) $(TEST_DIR)/$(FILE) src/pgv_utils.c $(TEST_CFLAGS) $(TEST_LIBS) -o $(TEST_BIN)
endif
	./$(TEST_BIN)


CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic

OBJS = src/i2c.o \
	src/mlx90614.o \
	src/sht31.o \
	src/cwsi.o \
	src/filter.o \
	src/validity.o \
	src/logger.o \
	src/config.o \
	src/display.o

BIN = cwsi_monitor

.SUFFIXES: .c .o

.PHONY: all test clean

all: $(BIN)

$(BIN): src/main.o $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -lm

# TESTING
TEST_SRCS = \
	test/test_cwsi.c \
	test/test_filter.c \
	test/test_validity.c \
	test/test_config.c \
	test/test_util.c \
	test/test_main.c

TEST_DEPS = \
	src/cwsi.c \
	src/i2c.c \
	src/filter.c \
	src/validity.c \
	src/config.c

TEST_BIN = test/test_runner

# build then run the whole testsuite
test: $(TEST_BIN)
	@./$(TEST_BIN)

$(TEST_BIN): $(TEST_SRCS) $(TEST_DEPS) test/test_util.h test/tests.h
	$(CC) $(CFLAGS) -Itest -Isrc $(TEST_SRCS) $(TEST_DEPS) -o $@ -lm

clean:
	$(RM) src/*.o $(BIN) $(TEST_BIN)


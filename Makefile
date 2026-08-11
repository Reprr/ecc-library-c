CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I./include

SRC_DIR = src
TEST_DIR = test
INCLUDE_DIR = include/ecc

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:.c=.o)

UNITY_SRC = $(TEST_DIR)/unity.c

TESTS = big_int_test field_test ecdh_api_test

all: $(TESTS)
s
big_int_test: $(TEST_DIR)/big_int_test.c $(UNITY_SRC) $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

field_test: $(TEST_DIR)/field_test.c $(UNITY_SRC) $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

ecdh_api_test: $(TEST_DIR)/ecdh_api_test.c $(UNITY_SRC) $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

test: $(TESTS)
	@echo "Running big_int_test..."
	@./big_int_test
	@echo ""
	@echo "Running field_test..."
	@./field_test
	@echo ""
	@echo "Running ecdh_test..."
	@./ecdh_api_test

clean:
	rm -f $(TESTS) $(OBJS)

.PHONY: all test clean

CC     := gcc
CFLAGS := -Wall -Wextra -std=c99 -g -O2 -MMD -MP
CFLAGS += -Iinclude -Iinclude/ecc
LDLIBS := -lm

ifdef ASAN
CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
LDLIBS += -fsanitize=address,undefined
endif

BUILD_DIR := build
OBJ_DIR   := $(BUILD_DIR)/obj
LIB_DIR   := $(BUILD_DIR)/lib
BIN_DIR   := $(BUILD_DIR)/bin

LIB_SRCS := $(wildcard src/*.c)
LIB_OBJS := $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(LIB_SRCS))
LIB_PIC  := $(patsubst src/%.c,$(OBJ_DIR)/%.pic.o,$(LIB_SRCS))

STATIC_LIB := $(LIB_DIR)/libecc.a
SHARED_LIB := $(LIB_DIR)/libecc.so

TEST_SRCS := $(wildcard test/*_test.c)
TEST_BINS := $(patsubst test/%_test.c,$(BIN_DIR)/%_test,$(TEST_SRCS))
UNITY_OBJ := $(OBJ_DIR)/unity.o

EXAMPLE_BIN := $(BIN_DIR)/ecdh_example

.PHONY: all libs shared test example run-tests clean help

all: libs test example

libs: $(STATIC_LIB)
shared: $(SHARED_LIB)
test: $(TEST_BINS)
example: $(EXAMPLE_BIN)

run-tests: $(TEST_BINS)
	@for t in $(TEST_BINS); do \
		echo "=== $$t ==="; $$t || exit 1; echo; \
	done
	@echo "All tests passed"

help:
	@echo "  make            - статическая библиотека + тесты + пример"
	@echo "  make shared     - динамическая библиотека libecc.so"
	@echo "  make run-tests  - прогнать все тесты"
	@echo "  make ASAN=1 ... - сборка с ASan/UBSan"
	@echo "  make clean      - очистка"

clean:
	rm -rf $(BUILD_DIR)



$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.pic.o: src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

$(STATIC_LIB): $(LIB_OBJS) | $(LIB_DIR)
	ar rcs $@ $^

$(SHARED_LIB): $(LIB_PIC) | $(LIB_DIR)
	$(CC) -shared $(CFLAGS) $^ -o $@ $(LDLIBS)

$(UNITY_OBJ): test/unity.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -Itest -c $< -o $@

$(OBJ_DIR)/%_test.o: test/%_test.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -Itest -c $< -o $@

$(BIN_DIR)/%_test: $(OBJ_DIR)/%_test.o $(UNITY_OBJ) $(STATIC_LIB) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(OBJ_DIR)/example.o: examples/ecdh_example.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(EXAMPLE_BIN): $(OBJ_DIR)/example.o $(STATIC_LIB) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(OBJ_DIR) $(LIB_DIR) $(BIN_DIR):
	mkdir -p $@

-include $(wildcard $(OBJ_DIR)/*.d)
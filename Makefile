# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -Itest -g
LDFLAGS =

# Директории
SRC_DIR = src
TEST_DIR = test
ECDH_DIR = ecdh
INCLUDE_DIR = include

# -------------------------------------------------------------------
# 1. Сборка основной программы ECDH (использует и big_int, и field)
# -------------------------------------------------------------------
ECDH_SRCS = $(ECDH_DIR)/main.c $(SRC_DIR)/big_int.c $(SRC_DIR)/field.c
ECDH_OBJS = $(ECDH_SRCS:.c=.o)

# -------------------------------------------------------------------
# 2. Сборка теста для big_int (БЕЗ field.c)
# -------------------------------------------------------------------
BIGINT_TEST_SRCS = $(TEST_DIR)/big_int_test.c $(TEST_DIR)/unity.c $(SRC_DIR)/big_int.c
BIGINT_TEST_OBJS = $(BIGINT_TEST_SRCS:.c=.o)

# -------------------------------------------------------------------
# Цели по умолчанию
# -------------------------------------------------------------------
.PHONY: all clean test test-bigint

all: ecdh test-bigint   # собирает и прогу, и тест

# Сборка основной программы
ecdh: $(ECDH_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Сборка и запуск теста big_int
test-bigint: $(BIGINT_TEST_OBJS)
	$(CC) $(CFLAGS) -o test_bigint $^ $(LDFLAGS)
	./test_bigint

# Синоним для удобства
test: test-bigint

# -------------------------------------------------------------------
# Общее правило компиляции .c -> .o (работает для файлов в любых папках)
# -------------------------------------------------------------------
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# -------------------------------------------------------------------
# Явные зависимости от заголовков (для пересборки при их изменении)
# -------------------------------------------------------------------
$(SRC_DIR)/big_int.o: $(INCLUDE_DIR)/ecc/big_int.h $(INCLUDE_DIR)/ecc/types.h
$(SRC_DIR)/field.o: $(INCLUDE_DIR)/ecc/field.h $(INCLUDE_DIR)/ecc/types.h
$(TEST_DIR)/big_int_test.o: $(INCLUDE_DIR)/ecc/big_int.h $(INCLUDE_DIR)/ecc/types.h $(TEST_DIR)/unity.h
$(TEST_DIR)/unity.o: $(TEST_DIR)/unity.h
$(ECDH_DIR)/main.o: $(INCLUDE_DIR)/ecc/big_int.h $(INCLUDE_DIR)/ecc/field.h $(INCLUDE_DIR)/ecc/types.h

# -------------------------------------------------------------------
# Очистка
# -------------------------------------------------------------------
clean:
	rm -f $(SRC_DIR)/*.o $(TEST_DIR)/*.o $(ECDH_DIR)/*.o ecdh test_bigint
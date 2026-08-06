# Компилятор и флаги
CC       = gcc
CFLAGS   = -Wall -Wextra -std=c99 -g -O2
INCLUDES = -Iinclude -Iinclude/ecc -Itest
CFLAGS   += $(INCLUDES) -MMD -MP
LDFLAGS  = -lm

# Директории сборки
BUILD_DIR = build
OBJ_DIR   = $(BUILD_DIR)/objs
LIB_DIR   = $(BUILD_DIR)/lib
BIN_DIR   = $(BUILD_DIR)/bin

# Имена конечных файлов
LIBRARY      = libecc.a
APP_EXEC     = ecdh          # основное приложение
TEST_EXEC    = test_runner

LIBRARY_PATH = $(LIB_DIR)/$(LIBRARY)
APP_EXEC_PATH = $(BIN_DIR)/$(APP_EXEC)
TEST_EXEC_PATH = $(BIN_DIR)/$(TEST_EXEC)

# Исходники библиотеки и приложения
LIB_SRCS  = src/big_int.c src/field.c
APP_SRCS  = main.c           # лежит в корне

# Исходники тестов (без main.c, потому что big_int_test.c уже содержит main)
TEST_SRCS = test/big_int_test.c test/unity.c

# Объектные файлы
LIB_OBJS  = $(patsubst src/%.c, $(OBJ_DIR)/%.o, $(LIB_SRCS))
APP_OBJS  = $(OBJ_DIR)/main.o
TEST_OBJS = $(patsubst test/%.c, $(OBJ_DIR)/%.o, $(TEST_SRCS))

# Файлы зависимостей
DEPS = $(LIB_OBJS:.o=.d) $(APP_OBJS:.o=.d) $(TEST_OBJS:.o=.d)

# Цели по умолчанию
.PHONY: all app tests check clean distclean

all: app tests

app: $(APP_EXEC_PATH)

tests: $(TEST_EXEC_PATH)

check: tests
	./$(TEST_EXEC_PATH)

# --- Сборка библиотеки ---
$(LIBRARY_PATH): $(LIB_OBJS) | $(LIB_DIR)
	ar rcs $@ $^

# --- Сборка приложения ---
$(APP_EXEC_PATH): $(APP_OBJS) $(LIBRARY_PATH) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# --- Сборка тестов ---
$(TEST_EXEC_PATH): $(TEST_OBJS) $(LIBRARY_PATH) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# --- Правила компиляции ---
# Из src/
$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Из test/
$(OBJ_DIR)/%.o: test/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Из корня (для main.c)
$(OBJ_DIR)/main.o: main.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Создание директорий
$(OBJ_DIR) $(LIB_DIR) $(BIN_DIR):
	mkdir -p $@

# --- Очистка ---
clean:
	rm -rf $(BUILD_DIR)

distclean: clean

# Подключение зависимостей
-include $(DEPS)
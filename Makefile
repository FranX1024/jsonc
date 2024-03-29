TEST_DIR = ./source/test
SRC_DIR = ./source
BIN_DIR = ./bin

C_FILES = $(wildcard $(TEST_DIR)/*.c)

TESTS = $(patsubst $(TEST_DIR)/%.c,$(BIN_DIR)/%,$(C_FILES))

CC = gcc
AR = ar
CFLAGS = -Wall -Wextra

all: jsonc $(TESTS)
tests: $(TESTS)

jsonc:
	@echo "Compiling library"
	$(CC) $(CFLAGS) $(SRC_DIR)/utils.c -c -o $(SRC_DIR)/utils.o
	$(CC) $(CFLAGS) $(SRC_DIR)/datatypes.c -c -o $(SRC_DIR)/datatypes.o
	$(CC) $(CFLAGS) $(SRC_DIR)/parser.c -c -o $(SRC_DIR)/parser.o
	$(AR) rvs ./lib/libjsonc.a $(SRC_DIR)/utils.o $(SRC_DIR)/datatypes.o $(SRC_DIR)/parser.o
	@echo "done"

$(BIN_DIR)/%: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@ -L./lib -ljsonc

clean:
	rm -f $(BIN_DIR)/* $(SRC_DIR)/*.o ./lib/libjsonc.a

.PHONY: all clean

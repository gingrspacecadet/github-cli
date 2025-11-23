CC = clang
DEBUG = true
CFLAGS += -Wall -Wextra -Werror -Wpedantic -g -Wno-unused-function -Iemu -std=gnu23 -O3
ifeq ($(DEBUG), true)
	CFLAGS += -DDEBUG
endif
LDFLAGS = -lcurl -lcjson

BUILD_DIR = build
SRC_DIR = src

SRC = $(shell find $(SRC_DIR) -name '*.c')
OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))

TARGET = build/gh

.PHONY: all clean run crun

all: $(TARGET)

$(TARGET): $(OBJ)
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	@rm -rf $(BUILD_DIR)
	@rm -f $(TARGET)
	@rm -f *.dump *.out

run: all
	@./$(TARGET) kernel.out bios.out

crun: clean run
CC = gcc
CFLAGS = -Wall -Wextra -I.

TARGET = poma_example
BUILD_DIR = build

OBJ = \
	$(BUILD_DIR)/main.o \
	$(BUILD_DIR)/poma_tcpconnector.o \
	$(BUILD_DIR)/poma_core.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Explicit header dependencies
$(BUILD_DIR)/main.o: main.c poma_tcpconnector.h | $(BUILD_DIR)

$(BUILD_DIR)/poma_tcpconnector.o: poma_tcpconnector.c poma_tcpconnector.h poma_core.h | $(BUILD_DIR)

$(BUILD_DIR)/poma_core.o: poma_core.c poma_core.h | $(BUILD_DIR)

# Generic rule for compilation into build directory
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean

# Makefile for A35M Cryptocurrency Prediction Model

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
INCLUDES = -I.
LIBS = -lssl -lcrypto -lm

# Directories
SRC_DIR = src
BUILD_DIR = build

# Source files
SOURCES = $(SRC_DIR)/main.c
OBJECTS = $(BUILD_DIR)/main.o
TARGET = btc_predict

# Default target
all: $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile source files
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c $(SRC_DIR)/include/models.h $(SRC_DIR)/include/openSSL.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Link the final executable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LIBS)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(TARGET) *.o

# Install (copy to system)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Uninstall
uninstall:
	rm -f /usr/local/bin/$(TARGET)

# Run with default BTC
run: $(TARGET)
	./$(TARGET)

# Run with ETH
run-eth: $(TARGET)
	./$(TARGET) ETH

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

# Help target
help:
	@echo "Available targets:"
	@echo "  all       - Build the project (default)"
	@echo "  clean     - Remove build artifacts"
	@echo "  debug     - Build with debug symbols"
	@echo "  run       - Build and run with BTC"
	@echo "  run-eth   - Build and run with ETH"
	@echo "  install   - Install to /usr/local/bin"
	@echo "  uninstall - Remove from /usr/local/bin"
	@echo "  help      - Show this help message"

.PHONY: all clean install uninstall run run-eth debug help

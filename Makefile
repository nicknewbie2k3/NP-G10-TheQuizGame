# Makefile for TCP Client-Server Framework

CC = gcc
CFLAGS = -Wall -Wextra -I./include
LDFLAGS = 

# Directories
SRC_DIR = src
INCLUDE_DIR = include
EXAMPLES_DIR = examples
BUILD_DIR = build

# Source files
SERVER_SRC = $(SRC_DIR)/tcp_server.c
CLIENT_SRC = $(SRC_DIR)/tcp_client.c

# Object files
SERVER_OBJ = $(BUILD_DIR)/tcp_server.o
CLIENT_OBJ = $(BUILD_DIR)/tcp_client.o

# Example programs
ECHO_SERVER = $(BUILD_DIR)/echo_server
ECHO_CLIENT = $(BUILD_DIR)/echo_client

# Targets
.PHONY: all clean examples

all: $(BUILD_DIR) examples

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build object files
$(SERVER_OBJ): $(SERVER_SRC) $(INCLUDE_DIR)/tcp_server.h
	$(CC) $(CFLAGS) -c $< -o $@

$(CLIENT_OBJ): $(CLIENT_SRC) $(INCLUDE_DIR)/tcp_client.h
	$(CC) $(CFLAGS) -c $< -o $@

# Build example programs
$(ECHO_SERVER): $(EXAMPLES_DIR)/echo_server.c $(SERVER_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(ECHO_CLIENT): $(EXAMPLES_DIR)/echo_client.c $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

examples: $(ECHO_SERVER) $(ECHO_CLIENT)
	@echo "Examples built successfully!"
	@echo "  Server: $(ECHO_SERVER)"
	@echo "  Client: $(ECHO_CLIENT)"

clean:
	rm -rf $(BUILD_DIR)
	@echo "Clean complete"

# Help target
help:
	@echo "Available targets:"
	@echo "  all       - Build all targets (default)"
	@echo "  examples  - Build example programs"
	@echo "  clean     - Remove all build artifacts"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make              # Build everything"
	@echo "  make examples     # Build example programs only"
	@echo "  make clean        # Clean build artifacts"

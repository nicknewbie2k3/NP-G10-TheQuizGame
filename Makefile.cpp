# Quiz Game C++ Server - Makefile
# Requires: libwebsockets, nlohmann-json

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
INCLUDES = -I/usr/include -I/usr/local/include -Icpp-server
LIBS = -lwebsockets -lpthread

# Directories
SRC_DIR = cpp-server
BUILD_DIR = build
PUBLIC_DIR = public
QUESTIONS_DIR = questions

# Source files
GAME_SERVER_SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/game_logic.cpp $(SRC_DIR)/json_loader.cpp
HTTP_SERVER_SOURCES = $(SRC_DIR)/http_server.cpp

# Output binaries
GAME_SERVER = $(BUILD_DIR)/game_server
HTTP_SERVER = $(BUILD_DIR)/http_server

# Phony targets
.PHONY: all clean game http install-deps copy-questions copy-public run-game run-http

# Default target
all: $(BUILD_DIR) $(GAME_SERVER) $(HTTP_SERVER) copy-questions copy-public

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build game server
$(GAME_SERVER): $(GAME_SERVER_SOURCES) $(SRC_DIR)/game_server.h
	@echo " Building WebSocket Game Server..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(GAME_SERVER_SOURCES) -o $(GAME_SERVER) $(LIBS)
	@echo " Game server built successfully!"

# Build HTTP server
$(HTTP_SERVER): $(HTTP_SERVER_SOURCES)
	@echo " Building HTTP Server..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(HTTP_SERVER_SOURCES) -o $(HTTP_SERVER) $(LIBS)
	@echo " HTTP server built successfully!"

# Copy question files
copy-questions:
	@echo " Copying question files..."
	@mkdir -p $(BUILD_DIR)/questions
	@if [ -d "websocket-bridge/questions" ]; then \
		cp websocket-bridge/questions/*.json $(BUILD_DIR)/questions/ 2>/dev/null || true; \
	fi
	@echo " Question files copied!"

# Copy public files
copy-public:
	@echo " Copying public web files..."
	@mkdir -p $(BUILD_DIR)/public
	@if [ -d "$(PUBLIC_DIR)" ]; then \
		cp -r $(PUBLIC_DIR)/* $(BUILD_DIR)/public/; \
	fi
	@echo " Public files copied!"

# Run game server
run-game: $(GAME_SERVER) copy-questions
	@echo " Starting WebSocket Game Server on port 8080..."
	@cd $(BUILD_DIR) && ./game_server

# Run HTTP server
run-http: $(HTTP_SERVER) copy-public
	@echo " Starting HTTP Server on port 3001..."
	@cd $(BUILD_DIR) && ./http_server

# Clean build artifacts
clean:
	@echo " Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)
	@echo " Clean complete!"

# Install dependencies (Ubuntu/Debian)
install-deps:
	@echo " Installing dependencies..."
	@echo "Note: This requires sudo privileges"
	sudo apt-get update
	sudo apt-get install -y \
		build-essential \
		cmake \
		libwebsockets-dev \
		nlohmann-json3-dev
	@echo " Dependencies installed!"

# Help
help:
	@echo " Quiz Game C++ Server - Build Commands"
	@echo ""
	@echo "Available targets:"
	@echo "  make all          - Build both servers (default)"
	@echo "  make game         - Build only game server"
	@echo "  make http         - Build only HTTP server"
	@echo "  make run-game     - Build and run game server"
	@echo "  make run-http     - Build and run HTTP server"
	@echo "  make clean        - Remove build artifacts"
	@echo "  make install-deps - Install required dependencies (Ubuntu/Debian)"
	@echo ""
	@echo " Required libraries:"
	@echo "  - libwebsockets"
	@echo "  - nlohmann-json"
	@echo ""
	@echo " Quick start:"
	@echo "  1. make install-deps  (first time only)"
	@echo "  2. make all"
	@echo "  3. In terminal 1: make run-game"
	@echo "  4. In terminal 2: make run-http"
	@echo "  5. Open browser: http://localhost:3001"


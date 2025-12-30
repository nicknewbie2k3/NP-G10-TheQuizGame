#!/bin/bash
# Linux Build and Run Script for Quiz Game

set -e

echo "🎮 Quiz Game C++ Server - Linux Build Script"
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Functions
print_success() {
    echo -e "${GREEN}$1${NC}"
}

print_error() {
    echo -e "${RED}$1${NC}"
}

print_info() {
    echo -e "${CYAN}$1${NC}"
}

print_warning() {
    echo -e "${YELLOW}$1${NC}"
}

# Check if running in WSL
if grep -qi microsoft /proc/version; then
    print_info "🐧 Running in WSL (Windows Subsystem for Linux)"
    IN_WSL=true
else
    print_info "🐧 Running in native Linux"
    IN_WSL=false
fi

# Check dependencies
check_dependencies() {
    print_info "📦 Checking dependencies..."
    
    if ! command -v g++ &> /dev/null; then
        print_error "❌ g++ not found! Please install: sudo apt-get install build-essential"
        exit 1
    fi
    
    if ! command -v cmake &> /dev/null; then
        print_error "❌ cmake not found! Please install: sudo apt-get install cmake"
        exit 1
    fi
    
    if ! pkg-config --exists libwebsockets 2>/dev/null; then
        print_error "❌ libwebsockets not found! Please install: sudo apt-get install libwebsockets-dev"
        exit 1
    fi
    
    if [ ! -f "/usr/include/nlohmann/json.hpp" ]; then
        print_error "❌ nlohmann-json not found! Please install: sudo apt-get install nlohmann-json3-dev"
        exit 1
    fi
    
    print_success "✅ All dependencies found!"
}

# Install dependencies
install_deps() {
    print_warning "📦 Installing dependencies..."
    sudo apt-get update
    sudo apt-get install -y build-essential cmake libwebsockets-dev nlohmann-json3-dev
    print_success "✅ Dependencies installed!"
}

# Build project
build_project() {
    print_warning "🔨 Building project..."
    make -f Makefile.cpp all
    print_success "✅ Build complete!"
}

# Clean build
clean_build() {
    print_warning "🧹 Cleaning build directory..."
    make -f Makefile.cpp clean
    print_success "✅ Clean complete!"
}

# Run servers
run_servers() {
    print_info "🚀 Starting servers..."
    
    # Check if servers exist
    if [ ! -f "build/game_server" ] || [ ! -f "build/http_server" ]; then
        print_error "❌ Servers not built! Run: ./linux-run.sh build"
        exit 1
    fi
    
    # Kill existing servers on ports
    print_info "Checking for existing processes on ports 8080 and 3001..."
    fuser -k 8080/tcp 2>/dev/null || true
    fuser -k 3001/tcp 2>/dev/null || true
    sleep 1
    
    # Start game server in background
    print_info "Starting WebSocket Game Server on port 8080..."
    cd build
    ./game_server > game_server.log 2>&1 &
    GAME_PID=$!
    echo $GAME_PID > game_server.pid
    cd ..
    
    sleep 2
    
    # Start HTTP server in background
    print_info "Starting HTTP Server on port 3001..."
    cd build
    ./http_server > http_server.log 2>&1 &
    HTTP_PID=$!
    echo $HTTP_PID > http_server.pid
    cd ..
    
    sleep 2
    
    # Check if servers are running
    if ps -p $GAME_PID > /dev/null && ps -p $HTTP_PID > /dev/null; then
        print_success "✅ Both servers started successfully!"
        echo ""
        print_info "🎮 Game Server PID: $GAME_PID (ws://localhost:8080)"
        print_info "🌐 HTTP Server PID: $HTTP_PID (http://localhost:3001)"
        echo ""
        print_info "📝 Logs:"
        print_info "   Game Server: build/game_server.log"
        print_info "   HTTP Server: build/http_server.log"
        echo ""
        print_warning "To stop servers: ./linux-run.sh stop"
        print_warning "To view logs: tail -f build/game_server.log"
        echo ""
        
        if [ "$IN_WSL" = true ]; then
            print_info "🌐 Open browser in Windows to: http://localhost:3001"
        else
            print_info "🌐 Open browser to: http://localhost:3001"
            if command -v xdg-open &> /dev/null; then
                xdg-open "http://localhost:3001" 2>/dev/null || true
            fi
        fi
    else
        print_error "❌ Failed to start servers"
        print_info "Check logs in build/ directory"
        exit 1
    fi
}

# Stop servers
stop_servers() {
    print_warning "⏹️  Stopping servers..."
    
    if [ -f "build/game_server.pid" ]; then
        kill $(cat build/game_server.pid) 2>/dev/null || true
        rm build/game_server.pid
        print_success "✅ Game server stopped"
    fi
    
    if [ -f "build/http_server.pid" ]; then
        kill $(cat build/http_server.pid) 2>/dev/null || true
        rm build/http_server.pid
        print_success "✅ HTTP server stopped"
    fi
    
    # Ensure ports are free
    fuser -k 8080/tcp 2>/dev/null || true
    fuser -k 3001/tcp 2>/dev/null || true
    
    print_success "✅ Servers stopped"
}

# Show logs
show_logs() {
    if [ -f "build/game_server.log" ]; then
        print_info "📄 Game Server Log:"
        tail -n 20 build/game_server.log
    fi
    
    echo ""
    
    if [ -f "build/http_server.log" ]; then
        print_info "📄 HTTP Server Log:"
        tail -n 20 build/http_server.log
    fi
}

# Main script
case "$1" in
    install-deps)
        install_deps
        ;;
    check)
        check_dependencies
        ;;
    build)
        check_dependencies
        build_project
        ;;
    clean)
        clean_build
        ;;
    run)
        run_servers
        ;;
    stop)
        stop_servers
        ;;
    logs)
        show_logs
        ;;
    restart)
        stop_servers
        run_servers
        ;;
    all)
        check_dependencies
        build_project
        run_servers
        ;;
    *)
        echo "Usage: $0 {install-deps|check|build|clean|run|stop|logs|restart|all}"
        echo ""
        echo "Commands:"
        echo "  install-deps  - Install required dependencies"
        echo "  check         - Check if dependencies are installed"
        echo "  build         - Build the project"
        echo "  clean         - Clean build directory"
        echo "  run           - Run both servers"
        echo "  stop          - Stop running servers"
        echo "  logs          - Show server logs"
        echo "  restart       - Stop and restart servers"
        echo "  all           - Build and run everything"
        echo ""
        echo "Examples:"
        echo "  $0 install-deps  # First time setup"
        echo "  $0 all           # Build and run"
        echo "  $0 stop          # Stop servers"
        exit 1
        ;;
esac

exit 0

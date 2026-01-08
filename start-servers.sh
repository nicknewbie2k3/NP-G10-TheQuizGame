#!/bin/bash

# Safe server startup script that handles high PIDs
# This script ensures servers start cleanly by clearing port bindings

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "🔧 Cleaning up old processes..."

# Kill any existing game_server or http_server processes
pkill -f game_server || true
pkill -f http_server || true

# Also kill anything listening on our ports
fuser -k 8080/tcp 2>/dev/null || true
fuser -k 3001/tcp 2>/dev/null || true

# Give system time to clean up
sleep 2

echo "✅ Cleanup complete"
echo ""
echo "🚀 Starting servers..."

cd build

# Start game server
echo "Starting Game Server on port 8080..."
nohup ./game_server > game_server.log 2>&1 &
GAME_PID=$!
echo $GAME_PID > game_server.pid
echo "Game Server PID: $GAME_PID"

sleep 2

# Start HTTP server
echo "Starting HTTP Server on port 3001..."
nohup ./http_server > http_server.log 2>&1 &
HTTP_PID=$!
echo $HTTP_PID > http_server.pid
echo "HTTP Server PID: $HTTP_PID"

sleep 2

# Verify servers started
if [ -f game_server.log ] && ! grep -q "Failed to create libwebsocket context" game_server.log; then
    echo ""
    echo "✅ Servers started successfully!"
    echo "   Game Server: ws://localhost:8080 (PID: $GAME_PID)"
    echo "   HTTP Server: http://localhost:3001 (PID: $HTTP_PID)"
    echo ""
    echo "📝 Logs:"
    echo "   tail -f game_server.log"
    echo "   tail -f http_server.log"
else
    echo ""
    echo "❌ Server startup failed. Check logs:"
    tail -n 20 game_server.log
    exit 1
fi

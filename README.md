# NP-G10-TheQuizGame
Network Programming Project of Group 10.

A multiplayer quiz game with **3-round elimination system** and real-time gameplay featuring a **pure C++** implementation with WebSocket server and web-based frontend.

## 🎮 Project Overview

This project is a complete **C++ implementation** of a multiplayer quiz game with real-time gameplay:

- **Backend**: C++ WebSocket server using libwebsockets
- **Game Logic**: Pure C++ game state management and elimination system
- **Frontend**: Vanilla HTML/CSS/JavaScript (no frameworks)
- **HTTP Server**: C++ static file server for web interface
- **Gameplay**: 3-round elimination tournament with PIN-based room system

**📌 100% C/C++ Implementation** - No Node.js, no React, no external frameworks required!

## 🏆 Game Features

**Tournament Structure**:
- **Round 1**: All players (4+) compete → Lowest scorer eliminated
- **Round 2**: Remaining players → Another elimination  
- **Round 3**: Final 2 players → Winner determined

**Player Experience**:
- ✅ **Active players**: See questions and round progress
- ❌ **Eliminated players**: Get elimination screen + spectator mode
- 🍿 **Spectator mode**: Watch remaining players compete

## 🚀 Quick Start

### Prerequisites
- **Linux/WSL**: GCC/Clang with C++17 support
- **libwebsockets**: WebSocket library
- **nlohmann-json**: JSON parsing library (header-only)

> **💡 Windows Users**: Use WSL (Windows Subsystem for Linux)

### 1. Install Dependencies

**Ubuntu/Debian/WSL:**
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libwebsockets-dev nlohmann-json3-dev
```

### 2. Build the Project

```bash
make -f Makefile.cpp all
```

### 3. Run the Servers

**You need BOTH servers running simultaneously:**

**Option 1: From PowerShell (Windows)**
```powershell
# Start Game Server (WebSocket on port 8080)
wsl -d Ubuntu-24.04 -- bash -c "cd /mnt/d/Chinhphuc/NP-G10-TheQuizGame/build && nohup ./game_server > game_server.log 2>&1 & echo \`$! > game_server.pid"

# Start HTTP Server (on port 3001)
wsl -d Ubuntu-24.04 -- bash -c "cd /mnt/d/Chinhphuc/NP-G10-TheQuizGame/build && nohup ./http_server > http_server.log 2>&1 & echo \`$! > http_server.pid"
```

**Option 2: From WSL Terminal (Recommended)**
```bash
# Navigate to build directory
cd /mnt/d/Chinhphuc/NP-G10-TheQuizGame/build

# Start Game Server (runs in background)
nohup ./game_server > game_server.log 2>&1 & echo $! > game_server.pid

# Start HTTP Server (runs in background)
nohup ./http_server > http_server.log 2>&1 & echo $! > http_server.pid
```

**Check if servers are running:**
```bash
ps aux | grep -E '(game_server|http_server)' | grep -v grep
```

**Stop the servers:**
```bash
# Stop Game Server
kill $(cat game_server.pid) && rm game_server.pid

# Stop HTTP Server
kill $(cat http_server.pid) && rm http_server.pid
```

### 4. Play the Game
1. Open browser to `http://localhost:3001`
2. **Host**: Click "Host a Game" → Get PIN
3. **Players**: Click "Join Game" → Enter PIN and name
4. **Host**: Click "Start Game" when ready (minimum 2 players)
5. Enjoy the quiz game!


## 📁 Project Structure

```
NP-G10-TheQuizGame/
├── cpp-server/              # C++ server implementation
│   ├── game_server.h       # Game structures and declarations
│   ├── main.cpp            # WebSocket server entry point
│   ├── game_logic.cpp      # Game state and message handlers
│   ├── json_loader.cpp     # JSON question file loader
│   ├── http_server.cpp     # HTTP static file server
│   └── json.hpp            # nlohmann/json library
│
├── public/                  # Web frontend
│   ├── index.html          # Single-page application
│   ├── styles.css          # Game styling
│   └── game.js             # Client-side game logic
│
├── websocket-bridge/questions/  # Question data
│   ├── round1-questions.json
│   ├── round2-question-packs.json
│   └── speed-questions.json
│
├── build/                   # Build output (generated)
│   ├── game_server         # WebSocket server binary
│   ├── http_server         # HTTP server binary
│   ├── questions/          # Copied question files
│   └── public/             # Copied web files
│
├── docs/                    # Documentation
├── CMakeLists.txt          # CMake build configuration
├── Makefile.cpp            # GNU Make build file
└── README.md               # This file
```

## 📝 Customizing Questions

Edit JSON files in `websocket-bridge/questions/` then rebuild:

```bash
make -f Makefile.cpp all
```

**Round 1 Questions** (`round1-questions.json`):
```json
{
  "id": 1,
  "text": "What is the capital of France?",
  "options": ["London", "Berlin", "Paris", "Madrid"],
  "correctAnswer": 2,
  "timeLimit": 15
}
```

**Round 2 Question Packs** (`round2-question-packs.json`):
```json
{
  "id": "pack1",
  "title": "🌍 Geography Masters",
  "description": "World capitals and landmarks",
  "questions": [
    {"id": "geo1", "text": "Capital of Australia?", "answer": "Canberra"}
  ]
}
```

## 🐛 Troubleshooting

**Servers not starting:**
```bash
# Check if ports are already in use
lsof -i :8080  # Game server port
lsof -i :3001  # HTTP server port

# Kill processes if needed
kill -9 <PID>
```

**Connection errors in browser:**
- Ensure BOTH servers are running
- Check browser console (F12) for errors
- Verify servers are on correct ports (8080 and 3001)

**Buttons not working:**
- Hard refresh the page (Ctrl+Shift+R)
- Clear browser cache
- Check browser console for JavaScript errors

## 📚 Documentation

- **[CPP_IMPLEMENTATION.md](docs/CPP_IMPLEMENTATION.md)** - Complete architecture details
- **[WINDOWS_BUILD.md](docs/WINDOWS_BUILD.md)** - Windows-specific build instructions
- **[CONVERSION_SUMMARY.md](docs/CONVERSION_SUMMARY.md)** - Migration from Node.js to C++

## 🛠️ Technology Stack

**Backend**: C++ with libwebsockets, nlohmann/json
**Frontend**: HTML5, CSS3, Vanilla JavaScript
**Protocol**: WebSocket for real-time communication

---

Made with ❤️ by Group 10 for Network Programming Course

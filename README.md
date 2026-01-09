# NP-G10-TheQuizGame
Network Programming Project of Group 10.

A multiplayer quiz game with **3-round elimination system** and real-time gameplay featuring a **pure C++** implementation with WebSocket server and web-based frontend.

> ** Important**: This project has been converted to pure C++. See [CONVERSION_SUMMARY.md](docs/CONVERSION_SUMMARY.md) for details.

##  Project Overview

This project is a complete **C++ implementation** of a multiplayer quiz game with real-time gameplay:

- **Backend**: C++ WebSocket server using libwebsockets
- **Game Logic**: Pure C++ game state management and elimination system
- **Frontend**: Vanilla HTML/CSS/JavaScript (no frameworks)
- **HTTP Server**: C++ static file server for web interface
- **Gameplay**: 3-round elimination tournament with PIN-based room system

** 100% C/C++ Implementation** - No Node.js, no React, no external frameworks required!

##  Elimination System

**Tournament Structure**:
- **Round 1**: All players (4+) compete  Lowest scorer eliminated
- **Round 2**: Remaining players  Another elimination  
- **Round 3**: Final 2 players  Winner determined

**Player Experience**:
-  **Active players**: See questions and round progress
-  **Eliminated players**: Get elimination screen + spectator mode
-  **Spectator mode**: Watch remaining players compete

##  Quick Start

### Prerequisites
- **Linux/macOS**: GCC/Clang with C++17 support
- **Windows**: WSL (Windows Subsystem for Linux) recommended
- **libwebsockets**: WebSocket library
- **nlohmann-json**: JSON parsing library (header-only)

> ** Windows Users**: Use WSL for best Linux compatibility.

### Installation & Setup (Complete Guide)

**Step 1: Install Dependencies**

**Ubuntu/Debian (including WSL):**
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libwebsockets-dev nlohmann-json3-dev
```

**macOS:**
```bash
brew install cmake libwebsockets nlohmann-json
```

**Step 2: Clone the Repository**
```bash
git clone https://github.com/nicknewbie2k3/NP-G10-TheQuizGame.git
cd NP-G10-TheQuizGame
```

**Step 3: Build the Project**
```bash
make -f Makefile.cpp all
```

This will:
- Compile the game server (WebSocket on port 8080)
- Compile the HTTP server (on port 3001)
- Copy question files to `build/questions/`
- Copy web files to `build/public/`

**Step 4: Run Both Servers**

**From WSL/Linux Terminal:**
```bash
cd build

# Terminal 1: Start Game Server
nohup ./game_server > game_server.log 2>&1 & echo $! > game_server.pid

# Terminal 2: Start HTTP Server  
nohup ./http_server > http_server.log 2>&1 & echo $! > http_server.pid
```

**From PowerShell (Windows):**
```powershell
# Terminal 1: Start Game Server
wsl -d Ubuntu-24.04 -- bash -c "cd /mnt/d/path/to/NP-G10-TheQuizGame/build && nohup ./game_server > game_server.log 2>&1 & echo \`$! > game_server.pid"

# Terminal 2: Start HTTP Server
wsl -d Ubuntu-24.04 -- bash -c "cd /mnt/d/path/to/NP-G10-TheQuizGame/build && nohup ./http_server > http_server.log 2>&1 & echo \`$! > http_server.pid"
```

**Step 5: Play the Game**

1. Open your browser to **`http://localhost:3001`**
2. **Host's Actions**:
   - Click " Host a Game"
   - Share the PIN with other players
   - Wait for players to join
   - Click " Start Game" (minimum 2 players)
3. **Players' Actions**:
   - Click " Join Game"
   - Enter the PIN and your name
   - Wait for host to start
4. **Play**:
   - **Round 1**: Answer multiple-choice questions (15 seconds each)
   - **Round 2**: Speed questions (no time limit, fastest answer wins)
   - **Round 3**: Final showdown between last 2 players

### Stopping the Servers

**From WSL/Linux:**
```bash
cd build
kill $(cat game_server.pid) && rm game_server.pid
kill $(cat http_server.pid) && rm http_server.pid
```

**Check Server Status:**
```bash
ps aux | grep -E '(game_server|http_server)' | grep -v grep
```

### Troubleshooting Build Issues

**Error: Command not found (game_server/http_server)**
- Make sure you're in the `build/` directory
- Run `make -f Makefile.cpp all` first to compile

**Error: libwebsockets not found**
```bash
sudo apt-get install libwebsockets-dev
```

**Error: nlohmann/json.hpp not found**
```bash
sudo apt-get install nlohmann-json3-dev
```

**Error: Port 8080 or 3001 already in use**
```bash
# Find process using the port
lsof -i :8080  # or :3001

# Kill the process
kill -9 <PID>
```

##  Project Structure

```
NP-G10-TheQuizGame/
 cpp-server/              # C++ server implementation 
    main.cpp            # WebSocket game server
    game_logic.cpp      # Game state management
    json_loader.cpp     # Question loader
    http_server.cpp     # HTTP static file server
    game_server.h       # Header file

 public/                  # Vanilla JS frontend 
    index.html          # Main HTML
    game.js             # Client logic
    styles.css          # Styling

 websocket-bridge/
    questions/          # Question JSON files 
        round1-questions.json
        round2-question-packs.json
        speed-questions.json

 build/                   # Build output (generated)
    game_server         # Compiled WebSocket server
    http_server         # Compiled HTTP server
    questions/          # Copied from websocket-bridge/questions/
    public/             # Copied from public/

 docs/                    # Documentation
 CMakeLists.txt          # CMake build config
 Makefile.cpp            # Make build config
 build.ps1               # Windows build script
```

##  Customizing Questions

The game loads questions from JSON configuration files. No code changes needed!

### Question Files Location
```
websocket-bridge/questions/   # Edit these source files
 round1-questions.json     # Round 1 multiple choice questions
 round2-question-packs.json # Round 2 question packs for turn-based play  
 speed-questions.json      # Speed questions for turn order determination
```

Questions are automatically copied to `build/questions/` during compilation.

### Modifying Round 1 Questions
Edit `round1-questions.json`:
```json
[
  {
    "id": 1,
    "text": "What is the capital of France?",
    "options": ["London", "Berlin", "Paris", "Madrid"],
    "correctAnswer": 2,
    "timeLimit": 15
  }
]
```

**Properties:**
- `id`: Unique question identifier (number)
- `text`: Question text displayed to players
- `options`: Array of 4 answer choices
- `correctAnswer`: Index of correct option (0-3)
- `timeLimit`: Seconds players have to answer

### Modifying Round 2 Question Packs
Edit `round2-question-packs.json`:
```json
[
  {
    "id": "pack1",
    "title": " Geography Masters",
    "description": "World capitals, countries, and landmarks",
    "questions": [
      {
        "id": "geo1", 
        "text": "What is the capital of Australia?", 
        "answer": "Canberra"
      }
    ]
  }
]
```

**Properties:**
- `id`: Unique pack identifier (string)
- `title`: Pack name with emoji (shown in selection)
- `description`: Brief description of pack theme

After editing, rebuild to copy updated files:
```bash
make -f Makefile.cpp copy-questions
# Or just rebuild: make -f Makefile.cpp all
```

##  Documentation

- **[CPP_IMPLEMENTATION.md](docs/CPP_IMPLEMENTATION.md)** - Complete architecture and implementation details
- **[WINDOWS_BUILD.md](docs/WINDOWS_BUILD.md)** - Windows-specific build instructions with PowerShell scripts
- **[MIGRATION_GUIDE.md](docs/MIGRATION_GUIDE.md)** - Guide for converting from Node.js/React to C++
- **[FRAMEWORK.md](docs/FRAMEWORK.md)** - Original TCP framework documentation

##  Technology Stack

### Backend (C++)
- **libwebsockets** - WebSocket protocol implementation
- **nlohmann/json** - JSON parsing (header-only library)
- **C++17 STL** - Standard library containers and utilities
- **pthread** - POSIX threads for concurrent connections

### Frontend
- **Pure HTML5** - No templating engines
- **Pure CSS3** - No preprocessors
- **Vanilla JavaScript** - No frameworks (React/Vue/Angular)
- **WebSocket API** - Native browser WebSocket support

##  Project Structure

```
NP-G10-TheQuizGame/
 cpp-server/              # C++ server implementation
    game_server.h       # Game structures and declarations
    main.cpp            # WebSocket server entry point
    game_logic.cpp      # Game state and message handlers
    json_loader.cpp     # JSON question file loader
    http_server.cpp     # HTTP static file server
    json.hpp            # nlohmann/json library

 public/                  # Web frontend
    index.html          # Single-page application
    styles.css          # Game styling
    game.js             # Client-side game logic

 websocket-bridge/questions/  # Question data
    round1-questions.json
    round2-question-packs.json
    speed-questions.json

 build/                   # Build output (generated)
    game_server         # WebSocket server binary
    http_server         # HTTP server binary
    questions/          # Copied question files
    public/             # Copied web files

 docs/                    # Documentation
    CPP_IMPLEMENTATION.md
    WINDOWS_BUILD.md
    MIGRATION_GUIDE.md
    FRAMEWORK.md

 CMakeLists.txt          # CMake build configuration
 Makefile.cpp            # GNU Make build file
 README.md               # This file
```

##  Features

### Implemented 
-  WebSocket-based real-time communication
-  PIN-based game rooms
-  Multiple concurrent games
-  Player join/disconnect handling
-  Round 1: Multiple choice questions
-  Round 2: Turn-based question packs
-  Elimination system
-  Score tracking
-  Question timer
-  Dynamic question loading from JSON
-  Responsive web interface
-  Host controls (start, next, end game)

### Planned Enhancements 
-  SSL/TLS support (wss://)
-  Persistent leaderboards (SQLite)
-  Player authentication
-  Admin panel
-  Game statistics
-  Sound effects

##  Development

### Building for Development
```bash
# Build with debug symbols
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Or with Make
make -f Makefile.cpp clean
make -f Makefile.cpp all
```

### Adding New Features

1. **Server-side**: Edit `cpp-server/game_logic.cpp`
2. **Client-side**: Edit `public/game.js`
3. **UI**: Edit `public/index.html` and `public/styles.css`
4. **Questions**: Edit JSON files in `websocket-bridge/questions/`

### Message Protocol

All WebSocket messages use JSON format:

```javascript
// Client  Server
{ "type": "create_game" }
{ "type": "join_game", "gamePin": "ABC123", "playerName": "John" }
{ "type": "submit_answer", "questionId": 1, "answer": 2 }

// Server  Client
{ "type": "game_created", "gamePin": "ABC123" }
{ "type": "new_question", "question": {...}, "round": 1 }
{ "type": "player_eliminated", "playerId": "xyz", "playerName": "John" }
```

##  Troubleshooting

### Port Already in Use
```bash
# Linux/macOS
lsof -i :8080
kill -9 <PID>

# Windows
netstat -ano | findstr :8080
taskkill /PID <PID> /F
```

### Build Errors
- **C++17 not supported**: Update compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **libwebsockets not found**: Install development package
- **nlohmann-json not found**: Install or download `json.hpp`

### Runtime Errors
- **Connection refused**: Ensure game_server is running on port 8080
- **404 errors**: Ensure http_server is running and `public/` files are copied
- **Questions not loading**: Check `build/questions/` directory exists

##  Performance

Typical resource usage:
- **Memory**: 5-10 MB per server
- **CPU**: <1% idle, 5-10% during active game
- **Network**: ~1-5 KB/s per player
- **Latency**: 1-3 ms message processing

Supports 100+ concurrent games on modest hardware.

##  Contributing

This is a student project for Network Programming course. Contributions welcome!
- `questions`: Array of 5 questions per pack
  - `id`: Unique question identifier
  - `text`: Question text (host reads to player)
  - `answer`: Correct answer (for host reference)

### Modifying Speed Questions
Edit `speed-questions.json`:
```json
[
  {
    "id": "speed1",
    "question": "What is 7  8?",
    "correctAnswer": "56"
  }
]
```

**Properties:**
- `id`: Unique identifier for speed question
- `question`: Question text displayed to players
- `correctAnswer`: Exact answer string (case-insensitive)

### Adding New Content
1. **Backup originals**: Copy existing JSON files
2. **Edit files**: Add/modify questions following the format
3. **Restart server**: Questions load on server startup
4. **Test changes**: Verify questions appear correctly in game

**Tips:**
- Use emojis in pack titles for visual appeal
- Keep questions concise and clear
- Test answer formats (especially for speed questions)
- Include variety in difficulty and topics
- Round 2 packs should have exactly 5 questions each

##  Architecture

```
React Frontend (WebSocket)  Node.js Bridge  C TCP Server
                                                    
- Player Interface          - Elimination Logic  - Network Logic  
- Host Controls            - Room Management     - Question Bank
- Real-time Updates        - Score Tracking      - TCP Protocol
```

##  Framework

This project uses **POSIX Sockets (BSD Sockets)** as the client-server framework for TCP communication on Linux.

### Framework Features

-  Native Linux support (no external dependencies)
-  Direct TCP protocol implementation  
-  Clean, easy-to-use API
-  React.js frontend with WebSocket support
-  Real-time multiplayer functionality
-  Comprehensive documentation and examples

### Quick Start

#### Backend (C TCP Server)
```bash
# Build the TCP server
make

# Run the echo server (Terminal 1)
./build/echo_server

# Run the echo client (Terminal 2)
./build/echo_client
```

#### Frontend (React.js Quiz Game)
```bash
# Install and start WebSocket bridge
cd websocket-bridge
npm install
npm start

# In another terminal, install and start React frontend
cd frontend
npm install  
npm start
```

#### Playing the Game
1. Open browser to `http://localhost:3000`
2. Click "Host a Game" to create a new game
3. Share the PIN with players
4. Players join using "Join Game" with the PIN
5. Host starts the game and controls progression
6. Players answer questions in real-time
7. See live scores and final rankings!

### Documentation

For detailed framework documentation, see [docs/FRAMEWORK.md](docs/FRAMEWORK.md)

### Project Structure

```
.
 frontend/              # React.js quiz game frontend
    src/
       components/    # React components (GameHost, PlayerGame, etc.)
       context/       # Game state management
       services/      # WebSocket communication
    public/            # Static assets
    package.json       # Frontend dependencies
 websocket-bridge/      # Node.js WebSocket bridge server
    server.js          # Main bridge server logic
    package.json       # Bridge server dependencies
 include/              # C header files (API definitions)
 src/             # Implementation files
 examples/        # Example programs
 docs/            # Documentation
 build/           # Build output (generated)
 Makefile         # Build configuration
 README.md        # This file
```

### Building

```bash
make              # Build all targets
make examples     # Build example programs only
make clean        # Clean build artifacts
make help         # Show available targets
```

### Requirements

- GCC compiler
- Make build tool
- Linux/Unix operating system

### Framework Selection Rationale

**POSIX Sockets** was chosen for this project because:

1. **Native to Linux**: No external dependencies required
2. **TCP Support**: Direct support for TCP protocol via SOCK_STREAM
3. **Standard**: Industry-standard API, well-documented
4. **Lightweight**: Minimal overhead, direct system call access
5. **Educational**: Fundamental to understanding network programming

For more details, see the [Framework Documentation](docs/FRAMEWORK.md).


# C++ Implementation Guide

## Architecture Overview

The quiz game has been completely rewritten in C++ to meet the requirement of no external frameworks beyond standard libraries and essential networking components.

### Component Architecture

```

                   Web Browser                       
         (Vanilla HTML/CSS/JavaScript)               

             
              HTTP (port 3001)
             
        
          HTTP Server (C++)  
          Serves static files
        
             
             WebSocket (port 8080)
             
        
         WebSocket Game Server (C++)
         - Game logic              
         - Player management       
         - Elimination system      
         - Question handling       
        
```

## File Structure

```
NP-G10-TheQuizGame/
 cpp-server/              # C++ server implementation
    game_server.h       # Header file with structures
    main.cpp            # WebSocket server entry point
    game_logic.cpp      # Game state management
    json_loader.cpp     # JSON question loader
    http_server.cpp     # HTTP static file server
    json.hpp            # nlohmann/json library

 public/                  # Web frontend (vanilla JS)
    index.html          # Main HTML file
    styles.css          # Styling
    game.js             # Game client logic

 websocket-bridge/questions/  # Question JSON files
    round1-questions.json
    round2-question-packs.json
    speed-questions.json

 build/                   # Build output directory
    game_server         # Compiled game server
    http_server         # Compiled HTTP server
    questions/          # Copied question files
    public/             # Copied web files

 CMakeLists.txt          # CMake build configuration
 Makefile.cpp            # GNU Make build file
 README.md               # Main documentation
```

## Technology Stack

### Backend (C++)
- **Language**: C++17
- **WebSocket Library**: libwebsockets (for real-time communication)
- **JSON Parsing**: nlohmann/json (header-only library)
- **Threading**: pthread (for concurrent connections)

### Frontend
- **Pure HTML5**: No templating engines
- **Pure CSS3**: No preprocessors (SASS/LESS)
- **Vanilla JavaScript**: No frameworks (React/Vue/Angular)
- **WebSocket API**: Native browser WebSocket

## Building on Different Platforms

### Linux (Ubuntu/Debian)

1. **Install dependencies:**
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libwebsockets-dev \
    nlohmann-json3-dev
```

2. **Build:**
```bash
# Using Make
make -f Makefile.cpp all

# Or using CMake
mkdir build && cd build
cmake ..
make
```

3. **Run:**
```bash
# Terminal 1
cd build
./game_server

# Terminal 2
cd build
./http_server
```

### macOS

1. **Install dependencies:**
```bash
brew install cmake libwebsockets nlohmann-json
```

2. **Build and run** (same as Linux)

### Windows

#### Option 1: MSYS2 (Recommended)

1. **Install MSYS2** from https://www.msys2.org/

2. **Open MSYS2 MinGW 64-bit terminal:**
```bash
pacman -S mingw-w64-x86_64-gcc \
          mingw-w64-x86_64-cmake \
          mingw-w64-x86_64-libwebsockets \
          mingw-w64-x86_64-nlohmann-json
```

3. **Build:**
```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

4. **Run:**
```bash
./game_server.exe    # Terminal 1
./http_server.exe    # Terminal 2
```

#### Option 2: Visual Studio

1. **Install Visual Studio 2019/2022** with C++ workload

2. **Install vcpkg:**
```powershell
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat
./vcpkg install libwebsockets nlohmann-json
```

3. **Build with CMake:**
```powershell
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build . --config Release
```

## Key Implementation Details

### WebSocket Protocol

The C++ server uses **libwebsockets** to implement the WebSocket protocol. Key features:

- **Connection Management**: Handles multiple concurrent connections
- **Message Parsing**: JSON-based protocol for all game messages
- **Broadcasting**: Efficient message distribution to all players in a game
- **State Synchronization**: Real-time game state updates

### Game Logic

All game logic is implemented in pure C++:

```cpp
// Example: Player structure
struct Player {
    std::string id;
    std::string name;
    struct lws* wsi;         // WebSocket connection
    bool connected;
    bool hasAnswered;
    bool isEliminated;
    int score;
    int roundScore;
};

// Example: Game structure
struct Game {
    std::string pin;
    struct lws* hostWsi;
    std::vector<std::shared_ptr<Player>> players;
    std::string gameState;
    int currentRound;
    // ... more fields
};
```

### JSON Question Loading

Questions are loaded from JSON files at server startup:

```cpp
bool loadQuestionsFromJSON(const std::string& filename, 
                          std::vector<Question>& questions) {
    std::ifstream file(filename);
    json j;
    file >> j;
    
    for (const auto& item : j) {
        Question q;
        q.id = item["id"];
        q.text = item["text"];
        q.options = item["options"].get<std::vector<std::string>>();
        // ...
        questions.push_back(q);
    }
    return true;
}
```

### HTTP Static File Server

Simple HTTP server to serve web files:

- **MIME Type Detection**: Proper content types for HTML/CSS/JS
- **File Serving**: Serves files from `public/` directory
- **Default Route**: `/` serves `index.html`

## Message Protocol

All WebSocket messages use JSON format:

### Client  Server
```json
{
    "type": "create_game"
}

{
    "type": "join_game",
    "gamePin": "ABC123",
    "playerName": "John"
}

{
    "type": "submit_answer",
    "questionId": 1,
    "answer": 2
}
```

### Server  Client
```json
{
    "type": "game_created",
    "gamePin": "ABC123"
}

{
    "type": "new_question",
    "question": {
        "id": 1,
        "text": "What is the capital of France?",
        "options": ["London", "Berlin", "Paris", "Madrid"],
        "timeLimit": 15
    },
    "questionNumber": 1,
    "totalQuestions": 5,
    "round": 1
}

{
    "type": "player_eliminated",
    "playerId": "xyz789",
    "playerName": "John",
    "reason": "Lowest score in Round 1"
}
```

## Performance Considerations

- **Non-blocking I/O**: libwebsockets handles asynchronous operations
- **Memory Management**: Smart pointers (`std::shared_ptr`) for safe memory handling
- **Efficient Broadcasting**: Single message composition, multiple sends
- **STL Containers**: `std::map`, `std::vector` for efficient data structures

## Troubleshooting

### Port Already in Use

```bash
# Find process using port 8080
lsof -i :8080          # Linux/macOS
netstat -ano | findstr :8080  # Windows

# Kill the process
kill -9 <PID>          # Linux/macOS
taskkill /PID <PID> /F # Windows
```

### Missing Dependencies

```bash
# Check if libwebsockets is installed
pkg-config --modversion libwebsockets

# Check if nlohmann-json is installed
find /usr -name "json.hpp" 2>/dev/null
```

### Build Errors

1. **C++17 not supported**: Update GCC/Clang to version 7+
2. **libwebsockets not found**: Install development package
3. **nlohmann-json not found**: Download single header file from GitHub

## Future Enhancements

Potential improvements while maintaining C++ requirement:

- **SSL/TLS**: Secure WebSocket connections (wss://)
- **Database**: SQLite for persistent leaderboards
- **Authentication**: Player accounts and sessions
- **Admin Panel**: Game management interface
- **Statistics**: Game analytics and reporting
- **Audio**: Sound effects (using native libraries)

## Advantages of C++ Implementation

1. **Performance**: Native code, no JavaScript interpreter overhead
2. **Memory Efficiency**: Direct control over memory allocation
3. **Type Safety**: Compile-time type checking
4. **Portability**: Compiles on Linux/macOS/Windows
5. **Learning**: Deep understanding of networking and systems programming
6. **No Runtime Dependencies**: Just compile and run, no npm/node needed

## References

- **libwebsockets**: https://libwebsockets.org/
- **nlohmann/json**: https://github.com/nlohmann/json
- **WebSocket Protocol**: RFC 6455
- **C++ Reference**: https://en.cppreference.com/


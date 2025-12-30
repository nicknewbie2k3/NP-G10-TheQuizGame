# Quick Reference Card

## 🚀 Commands Cheat Sheet

### Build Commands

```bash
# Using Make (recommended)
make -f Makefile.cpp all          # Build both servers
make -f Makefile.cpp clean        # Clean build
make -f Makefile.cpp run-game     # Build & run game server
make -f Makefile.cpp run-http     # Build & run HTTP server
make -f Makefile.cpp help         # Show all commands

# Using CMake
mkdir build && cd build
cmake ..                          # Configure
make                              # Build
make clean                        # Clean
```

### Run Commands

```bash
# Start game server (Terminal 1)
cd build
./game_server                     # Linux/macOS
./game_server.exe                 # Windows

# Start HTTP server (Terminal 2)
cd build
./http_server                     # Linux/macOS
./http_server.exe                 # Windows
```

### Install Dependencies

```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake libwebsockets-dev nlohmann-json3-dev

# macOS
brew install cmake libwebsockets nlohmann-json

# Windows (MSYS2)
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake \
          mingw-w64-x86_64-libwebsockets mingw-w64-x86_64-nlohmann-json
```

## 📁 Important Files

| File | Purpose |
|------|---------|
| `cpp-server/main.cpp` | WebSocket server entry point |
| `cpp-server/game_logic.cpp` | Game state and handlers |
| `cpp-server/game_server.h` | Data structures |
| `cpp-server/http_server.cpp` | Static file server |
| `public/index.html` | Web interface |
| `public/game.js` | Client logic |
| `public/styles.css` | Styling |
| `websocket-bridge/questions/*.json` | Question files |

## 🌐 Ports & URLs

| Service | Port | URL |
|---------|------|-----|
| WebSocket Server | 8080 | `ws://localhost:8080` |
| HTTP Server | 3001 | `http://localhost:3001` |

## 🎮 Game Flow

1. **Host** creates game → Gets PIN
2. **Players** join with PIN + name
3. **Host** starts game (2+ players required)
4. **Round 1**: Multiple choice questions
5. Lowest scorer eliminated
6. **Round 2**: Turn-based questions
7. Another elimination
8. Winner declared!

## 📝 Question File Format

### Round 1 Questions
```json
{
  "id": 1,
  "text": "Question text?",
  "options": ["A", "B", "C", "D"],
  "correctAnswer": 2,
  "timeLimit": 15
}
```

### Speed Questions
```json
{
  "id": "speed1",
  "question": "What is 7 × 8?",
  "correctAnswer": "56"
}
```

### Question Packs
```json
{
  "id": "pack1",
  "title": "🌍 Geography",
  "description": "World capitals",
  "questions": [
    {
      "id": "geo1",
      "text": "Capital of France?",
      "answer": "Paris"
    }
  ]
}
```

## 🔧 Common Issues & Fixes

| Problem | Solution |
|---------|----------|
| Port in use | Kill process: `lsof -i :8080` then `kill -9 <PID>` |
| Can't connect | Check both servers running, firewall settings |
| Build fails | Verify C++17 support, install dependencies |
| Questions not loading | Run `make copy-questions` or check `build/questions/` |
| 404 errors | Check `build/public/` has HTML/CSS/JS files |

## 📊 WebSocket Messages

### Client → Server
```javascript
{ "type": "create_game" }
{ "type": "join_game", "gamePin": "ABC123", "playerName": "John" }
{ "type": "start_game" }
{ "type": "submit_answer", "questionId": 1, "answer": 2 }
{ "type": "next_question" }
{ "type": "next_round" }
{ "type": "end_game" }
```

### Server → Client
```javascript
{ "type": "game_created", "gamePin": "ABC123" }
{ "type": "join_success", "playerId": "xyz", "playerName": "John" }
{ "type": "player_joined", "playerName": "Jane", "players": [...] }
{ "type": "game_started", "round": 1 }
{ "type": "new_question", "question": {...}, "round": 1 }
{ "type": "answer_received", "correct": true }
{ "type": "question_results", "correctAnswer": 2, "scores": {...} }
{ "type": "round_complete", "round": 1 }
{ "type": "player_eliminated", "playerId": "xyz", "playerName": "John" }
{ "type": "game_ended" }
{ "type": "error", "message": "Error message" }
```

## 🎯 Key Data Structures

### Player
```cpp
struct Player {
    std::string id;
    std::string name;
    struct lws* wsi;
    bool connected;
    bool hasAnswered;
    bool isEliminated;
    int score;
    int roundScore;
};
```

### Game
```cpp
struct Game {
    std::string pin;
    struct lws* hostWsi;
    std::vector<std::shared_ptr<Player>> players;
    std::string gameState;  // "lobby", "round1", "round2", "finished"
    int currentRound;
    int currentQuestion;
    // ... more fields
};
```

## 🛠️ Debugging

### Enable Verbose Logging
```cpp
// In main.cpp, change:
int logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO | LLL_DEBUG;
```

### Check Server Status
```bash
# Process running?
ps aux | grep game_server
ps aux | grep http_server

# Ports listening?
netstat -tuln | grep 8080
netstat -tuln | grep 3001

# Test WebSocket
wscat -c ws://localhost:8080
```

### Browser Console
Press F12 in browser to see:
- WebSocket connection status
- JavaScript errors
- Network traffic

## 📖 Documentation Links

- [Full README](../README.md)
- [C++ Implementation Guide](CPP_IMPLEMENTATION.md)
- [Windows Build Guide](WINDOWS_BUILD.md)
- [Migration Guide](MIGRATION_GUIDE.md)

## 💡 Tips

- **Development**: Build with `-DCMAKE_BUILD_TYPE=Debug` for debugging
- **Production**: Build with `-DCMAKE_BUILD_TYPE=Release` for performance
- **Testing**: Use 2+ browser tabs/windows for multiplayer testing
- **Custom Questions**: Edit JSON files, no rebuild needed for game server
- **Frontend Changes**: Rebuild HTTP server to copy updated files

## ⚡ Quick Start (One-liner)

```bash
# Linux/macOS
sudo apt-get install -y cmake libwebsockets-dev nlohmann-json3-dev && \
make -f Makefile.cpp all && \
cd build && ./game_server & ./http_server &

# Then open: http://localhost:3001
```

## 🎓 Learning Resources

- **libwebsockets**: https://libwebsockets.org/
- **nlohmann/json**: https://github.com/nlohmann/json
- **WebSocket RFC**: https://datatracker.ietf.org/doc/html/rfc6455
- **C++ Reference**: https://en.cppreference.com/

---

**Last Updated**: 2025-12-30  
**Version**: 2.0 (C++ Implementation)

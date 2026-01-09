# Running on Linux (WSL) from Windows

##  Your Setup is Complete!

Your project is now configured to run in **WSL (Windows Subsystem for Linux)** and is 100% Linux-compatible!

##  Quick Start

### From Windows PowerShell:

```powershell
# Run everything (recommended)
wsl -d Ubuntu-24.04 -- bash -c "cd /mnt/d/Chinhphuc/NP-G10-TheQuizGame && ./linux-run.sh all"

# Or build and run separately
wsl -d Ubuntu-24.04 -- bash -c "cd /mnt/d/Chinhphuc/NP-G10-TheQuizGame && ./linux-run.sh build"
wsl -d Ubuntu-24.04 -- bash -c "cd /mnt/d/Chinhphuc/NP-G10-TheQuizGame && ./linux-run.sh run"

# Stop servers
wsl -d Ubuntu-24.04 -- bash -c "cd /mnt/d/Chinhphuc/NP-G10-TheQuizGame && ./linux-run.sh stop"
```

### From WSL Ubuntu Terminal:

```bash
# Enter WSL
wsl -d Ubuntu-24.04

# Navigate to project
cd /mnt/d/Chinhphuc/NP-G10-TheQuizGame

# Run everything
./linux-run.sh all

# Or use individual commands
./linux-run.sh build    # Build only
./linux-run.sh run      # Run servers
./linux-run.sh stop     # Stop servers
./linux-run.sh logs     # View logs
```

##  Available Commands

```bash
./linux-run.sh install-deps  # Install dependencies (first time)
./linux-run.sh check         # Check dependencies
./linux-run.sh build         # Build project
./linux-run.sh clean         # Clean build
./linux-run.sh run           # Run servers
./linux-run.sh stop          # Stop servers
./linux-run.sh logs          # Show logs
./linux-run.sh restart       # Restart servers
./linux-run.sh all           # Build and run
```

##  Playing the Game

1. **Start servers** (one of):
   ```bash
   ./linux-run.sh all              # From WSL
   # OR from PowerShell:
   wsl -d Ubuntu-24.04 -- bash -c "cd /mnt/d/Chinhphuc/NP-G10-TheQuizGame && ./linux-run.sh all"
   ```

2. **Open browser in Windows** to: `http://localhost:3001`

3. **Host a game** or **Join game**

4. **Play!** 

##  Architecture

```

   Windows Browser           
   http://localhost:3001     

           
            HTTP/WebSocket
           

   WSL (Ubuntu Linux)        
                             
      
    Game Server :8080      
      
                             
      
    HTTP Server :3001      
      

```

##  Troubleshooting

### Issue: "Permission denied"
```bash
chmod +x linux-run.sh
```

### Issue: Ports already in use
```bash
./linux-run.sh stop  # Stop existing servers
# Or manually:
fuser -k 8080/tcp
fuser -k 3001/tcp
```

### Issue: Can't connect from browser
- Make sure both servers are running: `./linux-run.sh logs`
- Check firewall settings in Windows
- WSL should forward ports automatically

### Issue: Dependencies not found
```bash
./linux-run.sh install-deps
```

##  Development Workflow

### 1. Make code changes in Windows
Edit files normally in VS Code or your preferred editor

### 2. Build in WSL
```bash
wsl -d Ubuntu-24.04 -- bash -c "cd /mnt/d/Chinhphuc/NP-G10-TheQuizGame && ./linux-run.sh build"
```

### 3. Test
```bash
./linux-run.sh restart  # Restart servers with new build
```

##  For Pure Linux (No WSL)

If deploying to a real Linux server:

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install build-essential cmake libwebsockets-dev nlohmann-json3-dev

# Build
make -f Makefile.cpp all

# Run (two terminals)
cd build && ./game_server    # Terminal 1
cd build && ./http_server    # Terminal 2

# Or use the script
./linux-run.sh all
```

##  Log Files

When running with `./linux-run.sh run`, logs are saved to:
- `build/game_server.log` - WebSocket server logs
- `build/http_server.log` - HTTP server logs

View logs:
```bash
./linux-run.sh logs          # Show last 20 lines
tail -f build/game_server.log  # Follow game server log
tail -f build/http_server.log  # Follow HTTP server log
```

##  Benefits of WSL

 **True Linux environment** on Windows
 **Full compatibility** with Linux libraries
 **Easy testing** before deploying to real Linux server
 **Seamless integration** - access Windows files from Linux
 **Native performance** - nearly as fast as real Linux

##  What You've Achieved

-  Pure C++ implementation
-  libwebsockets for WebSocket protocol
-  Runs natively on Linux
-  Cross-platform code (Linux/macOS/Windows with WSL)
-  No Node.js, no React, no frameworks
-  Production-ready for Linux deployment

Your project is ready for submission and deployment on Linux servers! 


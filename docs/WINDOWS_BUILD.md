# Windows Build Guide

This guide provides step-by-step instructions for building the Quiz Game C++ server on Windows.

## Prerequisites

You need to install a C++ compiler and the required libraries. There are two recommended approaches:

## Option 1: MSYS2 (Recommended for Beginners)

MSYS2 provides a Unix-like environment on Windows with easy package management.

### 1. Install MSYS2

1. Download MSYS2 installer from: https://www.msys2.org/
2. Run the installer (e.g., `msys2-x86_64-20231026.exe`)
3. Install to `C:\msys64` (default location)
4. At the end of installation, check "Run MSYS2 now"

### 2. Update MSYS2

In the MSYS2 terminal that opens:
```bash
pacman -Syu
```

If it asks to close the terminal, close it and reopen "MSYS2 MSYS" from Start menu, then run:
```bash
pacman -Su
```

### 3. Install Development Tools

Close the MSYS2 MSYS terminal and open **MSYS2 MinGW 64-bit** from Start menu.

Install required packages:
```bash
pacman -S --needed base-devel \
    mingw-w64-x86_64-gcc \
    mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-make \
    mingw-w64-x86_64-libwebsockets \
    mingw-w64-x86_64-nlohmann-json
```

Type `Y` and press Enter when prompted.

### 4. Build the Project

Navigate to your project directory:
```bash
cd /c/Users/YourUsername/path/to/NP-G10-TheQuizGame
```

Create build directory and compile:
```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

### 5. Run the Servers

**Terminal 1 - Game Server:**
```bash
./game_server.exe
```

**Terminal 2 - HTTP Server** (open new MSYS2 MinGW 64-bit terminal):
```bash
cd /c/Users/YourUsername/path/to/NP-G10-TheQuizGame/build
./http_server.exe
```

### 6. Play the Game

Open your browser to: `http://localhost:3001`

---

## Option 2: Visual Studio with vcpkg

For developers familiar with Visual Studio.

### 1. Install Visual Studio

1. Download **Visual Studio 2022 Community** from: https://visualstudio.microsoft.com/
2. During installation, select:
   - "Desktop development with C++"
   - Make sure "CMake tools for Windows" is checked

### 2. Install vcpkg (Package Manager)

Open PowerShell or Command Prompt:

```powershell
# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Bootstrap vcpkg
.\bootstrap-vcpkg.bat

# Install required libraries
.\vcpkg install libwebsockets:x64-windows
.\vcpkg install nlohmann-json:x64-windows

# Integrate with Visual Studio
.\vcpkg integrate install
```

Note the path to vcpkg, you'll need it later.

### 3. Build with CMake

Open PowerShell in your project directory:

```powershell
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake -DCMAKE_TOOLCHAIN_FILE="C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake" ..

# Build
cmake --build . --config Release
```

### 4. Run the Servers

**PowerShell 1 - Game Server:**
```powershell
cd build\Release
.\game_server.exe
```

**PowerShell 2 - HTTP Server:**
```powershell
cd build\Release
.\http_server.exe
```

### 5. Play the Game

Open browser to: `http://localhost:3001`

---

## Option 3: Manual Library Installation

If you prefer not to use package managers.

### 1. Install Compiler

Download and install MinGW-w64 from:
https://www.mingw-w64.org/downloads/

### 2. Download libwebsockets

1. Go to: https://github.com/warmcat/libwebsockets/releases
2. Download source code (e.g., `libwebsockets-4.3.2.zip`)
3. Extract and build:

```bash
cd libwebsockets-4.3.2
mkdir build
cd build
cmake -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=C:/local ..
mingw32-make
mingw32-make install
```

### 3. Download nlohmann-json

1. Go to: https://github.com/nlohmann/json/releases
2. Download `json.hpp` (single header file)
3. Place it in `cpp-server/json.hpp`

### 4. Build the Project

```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:/local ..
mingw32-make
```

---

## Troubleshooting

### Issue: "cmake: command not found"

**Solution**: Add CMake to PATH or use full path to cmake.exe

### Issue: "libwebsockets not found"

**Solution**: 
- MSYS2: Run `pacman -S mingw-w64-x86_64-libwebsockets`
- vcpkg: Run `vcpkg install libwebsockets:x64-windows`

### Issue: Port 8080 or 3001 already in use

**Solution**:
```powershell
# Find process using the port
netstat -ano | findstr :8080

# Kill the process (replace PID with actual process ID)
taskkill /PID <PID> /F
```

### Issue: Firewall blocking connections

**Solution**:
1. Open Windows Defender Firewall
2. Click "Allow an app through firewall"
3. Add `game_server.exe` and `http_server.exe`

### Issue: "Permission denied" when running executables

**Solution**:
- Disable antivirus temporarily
- Or add build directory to antivirus exclusions

### Issue: Missing DLLs when running

**Solution** (MSYS2):
```bash
# Copy required DLLs to build directory
cp /mingw64/bin/libwebsockets.dll build/
cp /mingw64/bin/libgcc_s_seh-1.dll build/
cp /mingw64/bin/libstdc++-6.dll build/
cp /mingw64/bin/libwinpthread-1.dll build/
```

---

## PowerShell Build Script

Create a file `build.ps1` in your project root:

```powershell
# Quick build script for Windows

Write-Host "🔨 Building Quiz Game C++ Server..." -ForegroundColor Cyan

# Check if build directory exists
if (!(Test-Path -Path "build")) {
    New-Item -ItemType Directory -Path "build"
}

# Navigate to build directory
Set-Location build

# Configure with CMake
Write-Host "📦 Configuring with CMake..." -ForegroundColor Yellow
cmake -G "MinGW Makefiles" ..

if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ CMake configuration failed!" -ForegroundColor Red
    exit 1
}

# Build
Write-Host "🔧 Compiling..." -ForegroundColor Yellow
mingw32-make

if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ Build failed!" -ForegroundColor Red
    exit 1
}

Write-Host "✅ Build successful!" -ForegroundColor Green
Write-Host ""
Write-Host "To run the servers:" -ForegroundColor Cyan
Write-Host "  Terminal 1: cd build && ./game_server.exe"
Write-Host "  Terminal 2: cd build && ./http_server.exe"
Write-Host "  Browser: http://localhost:3001"

Set-Location ..
```

Run with:
```powershell
.\build.ps1
```

---

## Quick Start Script

Create `run.ps1`:

```powershell
# Quick start script - runs both servers

Write-Host "🚀 Starting Quiz Game Servers..." -ForegroundColor Cyan

# Start game server in new window
Start-Process powershell -ArgumentList "-NoExit", "-Command", "cd '$PSScriptRoot\build'; ./game_server.exe"

# Wait a moment
Start-Sleep -Seconds 2

# Start HTTP server in new window
Start-Process powershell -ArgumentList "-NoExit", "-Command", "cd '$PSScriptRoot\build'; ./http_server.exe"

# Wait a moment
Start-Sleep -Seconds 2

# Open browser
Start-Process "http://localhost:3001"

Write-Host "✅ Servers started!" -ForegroundColor Green
Write-Host "📱 Browser should open automatically" -ForegroundColor Cyan
Write-Host "   If not, navigate to: http://localhost:3001" -ForegroundColor Yellow
```

Run with:
```powershell
.\run.ps1
```

---

## Recommended Setup

For the best Windows development experience:

1. **Install MSYS2** - Easy package management
2. **Use VS Code** with C/C++ extension
3. **Windows Terminal** - Better terminal experience
4. **Configure VS Code** to use MSYS2's MinGW compiler

VS Code `tasks.json`:
```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build Game Server",
            "type": "shell",
            "command": "mingw32-make",
            "args": [],
            "options": {
                "cwd": "${workspaceFolder}/build"
            }
        }
    ]
}
```

---

## Next Steps

After successful build:

1. ✅ Both servers are running
2. ✅ Browser shows the game interface
3. 🎮 Try hosting a game and joining from another tab
4. 📝 Customize questions in `build/questions/` directory
5. 🎨 Modify frontend in `public/` directory

Happy coding! 🚀

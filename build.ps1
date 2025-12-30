# Quiz Game C++ Server - Build Script for Windows
# Requires MSYS2 with MinGW-w64 installed

param(
    [switch]$Clean,
    [switch]$Build,
    [switch]$Run,
    [switch]$All,
    [switch]$Help
)

$ErrorActionPreference = "Stop"

function Write-ColorOutput($ForegroundColor) {
    $fc = $host.UI.RawUI.ForegroundColor
    $host.UI.RawUI.ForegroundColor = $ForegroundColor
    if ($args) {
        Write-Output $args
    }
    $host.UI.RawUI.ForegroundColor = $fc
}

function Show-Help {
    Write-ColorOutput Cyan @"
🎮 Quiz Game C++ Build Script

Usage:
    .\build.ps1 -All          Build everything
    .\build.ps1 -Clean        Clean build directory
    .\build.ps1 -Build        Build project
    .\build.ps1 -Run          Run servers
    .\build.ps1 -Help         Show this help

Examples:
    .\build.ps1 -All          # Clean, build, and run
    .\build.ps1 -Clean -Build # Clean then build
    .\build.ps1 -Run          # Run existing build

Requirements:
    - MSYS2 with MinGW-w64
    - CMake
    - libwebsockets
    - nlohmann-json

Install dependencies:
    In MSYS2 MinGW 64-bit terminal:
    pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake \
              mingw-w64-x86_64-libwebsockets mingw-w64-x86_64-nlohmann-json
"@
}

function Clean-Build {
    Write-ColorOutput Yellow "🧹 Cleaning build directory..."
    
    if (Test-Path "build") {
        Remove-Item -Recurse -Force "build"
        Write-ColorOutput Green "✅ Build directory cleaned"
    } else {
        Write-ColorOutput Cyan "ℹ️ Build directory doesn't exist"
    }
}

function Build-Project {
    Write-ColorOutput Yellow "🔨 Building project..."
    
    # Create build directory
    if (!(Test-Path "build")) {
        New-Item -ItemType Directory -Path "build" | Out-Null
    }
    
    # Check for MSYS2 CMake
    $cmakePaths = @(
        "C:\msys64\mingw64\bin\cmake.exe",
        "C:\msys64\usr\bin\cmake.exe",
        (Get-Command cmake -ErrorAction SilentlyContinue).Source
    )
    
    $cmake = $null
    foreach ($path in $cmakePaths) {
        if ($path -and (Test-Path $path)) {
            $cmake = $path
            break
        }
    }
    
    if (!$cmake) {
        Write-ColorOutput Red "❌ CMake not found!"
        Write-ColorOutput Yellow "Please install CMake via MSYS2:"
        Write-ColorOutput Cyan "  pacman -S mingw-w64-x86_64-cmake"
        exit 1
    }
    
    Write-ColorOutput Cyan "Using CMake: $cmake"
    
    # Configure
    Push-Location "build"
    try {
        Write-ColorOutput Yellow "📦 Configuring with CMake..."
        & $cmake -G "MinGW Makefiles" .. 2>&1 | Tee-Object -Variable cmakeOutput
        
        if ($LASTEXITCODE -ne 0) {
            Write-ColorOutput Red "❌ CMake configuration failed!"
            Write-ColorOutput Yellow "Make sure you have all dependencies installed"
            Pop-Location
            exit 1
        }
        
        # Build
        Write-ColorOutput Yellow "🔧 Compiling..."
        & mingw32-make 2>&1 | Tee-Object -Variable makeOutput
        
        if ($LASTEXITCODE -ne 0) {
            Write-ColorOutput Red "❌ Build failed!"
            Pop-Location
            exit 1
        }
        
        Write-ColorOutput Green "✅ Build successful!"
        
    } finally {
        Pop-Location
    }
}

function Run-Servers {
    Write-ColorOutput Yellow "🚀 Starting servers..."
    
    # Check if executables exist
    if (!(Test-Path "build\game_server.exe")) {
        Write-ColorOutput Red "❌ game_server.exe not found!"
        Write-ColorOutput Yellow "Run: .\build.ps1 -Build"
        exit 1
    }
    
    if (!(Test-Path "build\http_server.exe")) {
        Write-ColorOutput Red "❌ http_server.exe not found!"
        Write-ColorOutput Yellow "Run: .\build.ps1 -Build"
        exit 1
    }
    
    # Start game server in new window
    Write-ColorOutput Cyan "Starting WebSocket Game Server (port 8080)..."
    Start-Process powershell -ArgumentList @(
        "-NoExit",
        "-Command",
        "cd '$PSScriptRoot\build'; Write-Host '🎮 Game Server Running' -ForegroundColor Green; ./game_server.exe"
    )
    
    Start-Sleep -Seconds 2
    
    # Start HTTP server in new window
    Write-ColorOutput Cyan "Starting HTTP Server (port 3001)..."
    Start-Process powershell -ArgumentList @(
        "-NoExit",
        "-Command",
        "cd '$PSScriptRoot\build'; Write-Host '🌐 HTTP Server Running' -ForegroundColor Green; ./http_server.exe"
    )
    
    Start-Sleep -Seconds 2
    
    # Open browser
    Write-ColorOutput Cyan "Opening browser..."
    Start-Process "http://localhost:3001"
    
    Write-ColorOutput Green @"

✅ Servers started successfully!

🎮 Game Server: ws://localhost:8080
🌐 Web Interface: http://localhost:3001

To stop servers: Close the PowerShell windows or press Ctrl+C
"@
}

# Main script logic
if ($Help -or (!$Clean -and !$Build -and !$Run -and !$All)) {
    Show-Help
    exit 0
}

if ($All) {
    Clean-Build
    Build-Project
    Run-Servers
    exit 0
}

if ($Clean) {
    Clean-Build
}

if ($Build) {
    Build-Project
}

if ($Run) {
    Run-Servers
}

Write-ColorOutput Green "✅ Done!"

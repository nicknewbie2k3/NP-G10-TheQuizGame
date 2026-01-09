#!/usr/bin/env pwsh

# Safe server startup script for Windows (PowerShell)
# This script cleans up old processes before starting servers

Write-Host " Cleaning up old processes..." -ForegroundColor Yellow

# Kill existing processes
Stop-Process -Name "game_server" -Force -ErrorAction SilentlyContinue | Out-Null
Stop-Process -Name "http_server" -Force -ErrorAction SilentlyContinue | Out-Null

# Kill processes on ports 8080 and 3001
$port8080 = Get-NetTCPConnection -LocalPort 8080 -ErrorAction SilentlyContinue | Where-Object {$_.State -eq "Listen"}
if ($port8080) {
    Stop-Process -Id $port8080.OwningProcess -Force -ErrorAction SilentlyContinue
}

$port3001 = Get-NetTCPConnection -LocalPort 3001 -ErrorAction SilentlyContinue | Where-Object {$_.State -eq "Listen"}
if ($port3001) {
    Stop-Process -Id $port3001.OwningProcess -Force -ErrorAction SilentlyContinue
}

Start-Sleep -Seconds 2

Write-Host " Cleanup complete" -ForegroundColor Green
Write-Host ""
Write-Host " Starting servers..." -ForegroundColor Yellow

Push-Location build

# Start game server
Write-Host "Starting Game Server on port 8080..."
$gameProcess = Start-Process -FilePath ".\game_server.exe" -RedirectStandardOutput "game_server.log" -RedirectStandardError "game_server_error.log" -PassThru -NoNewWindow
$gameProcess.Id | Out-File -FilePath "game_server.pid" -NoNewline
Write-Host "Game Server PID: $($gameProcess.Id)"

Start-Sleep -Seconds 2

# Start HTTP server  
Write-Host "Starting HTTP Server on port 3001..."
$httpProcess = Start-Process -FilePath ".\http_server.exe" -RedirectStandardOutput "http_server.log" -RedirectStandardError "http_server_error.log" -PassThru -NoNewWindow
$httpProcess.Id | Out-File -FilePath "http_server.pid" -NoNewline
Write-Host "HTTP Server PID: $($httpProcess.Id)"

Start-Sleep -Seconds 2

Pop-Location

# Check if servers are still running
if ($gameProcess.HasExited -eq $false -and $httpProcess.HasExited -eq $false) {
    Write-Host ""
    Write-Host " Servers started successfully!" -ForegroundColor Green
    Write-Host "   Game Server: ws://localhost:8080 (PID: $($gameProcess.Id))"
    Write-Host "   HTTP Server: http://localhost:3001 (PID: $($httpProcess.Id))"
    Write-Host ""
    Write-Host " Logs:"
    Write-Host "   build\game_server.log"
    Write-Host "   build\http_server.log"
} else {
    Write-Host ""
    Write-Host " Server startup failed. Check logs:" -ForegroundColor Red
    if ((Test-Path "build\game_server.log")) {
        Get-Content "build\game_server.log" -Tail 20
    }
    exit 1
}


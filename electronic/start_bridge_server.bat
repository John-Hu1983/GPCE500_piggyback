@echo off
start "EasyEDA Bridge Server" powershell.exe -NoExit -ExecutionPolicy Bypass -Command "Set-Location -LiteralPath \"$env:USERPROFILE\.copilot\skills\easyeda-api\"; Write-Host 'Starting EasyEDA Bridge Server...' -ForegroundColor Cyan; npm run server"

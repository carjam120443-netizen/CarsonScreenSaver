@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Install-CarsonScreenSavers-v2.ps1"
if errorlevel 1 (
    echo.
    echo Installation failed. Please run this file as Administrator.
    pause
    exit /b 1
)
endlocal

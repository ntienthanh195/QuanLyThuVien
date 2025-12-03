@echo off
REM ========================================
REM Quick Run Qt Library Management App
REM (Without rebuild)
REM ========================================

echo.
echo ====================================
echo Starting Qt Library Management...
echo ====================================
echo.

if not exist "release\LibraryManagement.exe" (
    echo ERROR: Application not built yet!
    echo Please run build-and-run.bat first
    pause
    exit /b 1
)

echo Launching application...
start "" "release\LibraryManagement.exe"

echo Application started!
echo.

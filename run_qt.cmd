@echo off
REM Script to run Qt Library Management application

echo Starting Library Management System...
echo.

REM Check if executable exists
if not exist "release\LibraryManagement.exe" (
    echo ERROR: LibraryManagement.exe not found in release directory!
    echo Please build the project first with:
    echo   C:\Qt\6.10.1\mingw_64\bin\qmake.exe LibraryManagement.pro
    echo   C:\Qt\Tools\mingw1310_64\bin\mingw32-make.exe
    pause
    exit /b 1
)

REM Add Qt DLLs to PATH
set PATH=C:\Qt\6.10.1\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;%PATH%

REM Run the application
cd /d "%~dp0"
start "" "release\LibraryManagement.exe"

echo Application started successfully!

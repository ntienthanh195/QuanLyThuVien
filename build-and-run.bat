@echo off
REM ========================================
REM Build and Run Qt Library Management App
REM ========================================

echo.
echo ====================================
echo Building Qt Library Management...
echo ====================================
echo.

REM Clean previous build
if exist "release\" (
    echo Cleaning previous build...
    rmdir /s /q release
)

if exist "debug\" (
    rmdir /s /q debug
)

REM Run qmake
echo Running qmake...
C:\Qt\6.10.1\mingw_64\bin\qmake.exe LibraryManagement.pro
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: qmake failed!
    pause
    exit /b 1
)

REM Build with mingw32-make
echo.
echo Building application...
C:\Qt\Tools\mingw1310_64\bin\mingw32-make.exe
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo ====================================
echo BUILD SUCCESSFUL!
echo ====================================
echo.

REM Run the application
echo Starting application...
start "" "release\LibraryManagement.exe"

echo Application launched!
echo.
pause

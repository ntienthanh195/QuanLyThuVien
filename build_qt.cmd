@echo off
REM Script to build Qt Library Management project

echo ========================================
echo Building Library Management System (Qt6)
echo ========================================
echo.

REM Set Qt paths
set QMAKE=C:\Qt\6.10.1\mingw_64\bin\qmake.exe
set MAKE=C:\Qt\Tools\mingw1310_64\bin\mingw32-make.exe

REM Check if qmake exists
if not exist "%QMAKE%" (
    echo ERROR: qmake not found at %QMAKE%
    echo Please update the path in this script.
    pause
    exit /b 1
)

REM Clean previous build
if exist "Makefile" (
    echo Cleaning previous build...
    if exist "%MAKE%" (
        "%MAKE%" clean
    )
    del /F /Q Makefile Makefile.Debug Makefile.Release .qmake.stash 2>nul
)

REM Generate Makefile
echo.
echo [1/2] Generating Makefile...
"%QMAKE%" LibraryManagement.pro
if errorlevel 1 (
    echo ERROR: qmake failed!
    pause
    exit /b 1
)

REM Build project
echo.
echo [2/2] Building project...
"%MAKE%"
if errorlevel 1 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo BUILD SUCCESSFUL!
echo ========================================
echo.
echo Executable: release\LibraryManagement.exe
echo Size: 
dir release\LibraryManagement.exe | find "LibraryManagement.exe"
echo.
echo To run the application:
echo   run_qt.cmd
echo.
pause

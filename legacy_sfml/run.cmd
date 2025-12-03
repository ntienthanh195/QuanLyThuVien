@echo off
rem run.cmd — build (if needed) and run the program in this workspace
pushd "%~dp0" || exit /b 1
setlocal

rem Use UTF-8 console for nice output
chcp 65001 >nul

rem Paths relative to repository root (script dir)
set "ROOT=%~dp0"
set "BIN=%ROOT%bin"
if not exist "%BIN%" mkdir "%BIN%" >nul 2>&1

set "SFML_BIN=%ROOT%lib\SFML\bin"
set "SFML_LIB=%ROOT%lib\SFML\lib"
set "SFML_INCLUDE=%ROOT%lib\SFML\include"

rem Try to find a MinGW-w64 g++ automatically (common locations)
set "GPP=C:\msys64\mingw64\bin\g++.exe"
if not exist "%GPP%" (
    if exist "C:\mingw64\bin\g++.exe" set "GPP=C:\mingw64\bin\g++.exe"
)
if not exist "%GPP%" (
    where g++ >nul 2>&1 && for /f "delims=" %%p in ('where g++') do set "GPP=%%p" && goto :GPP_FOUND
)
:GPP_FOUND
if not exist "%GPP%" (
    echo ❌ No g++ compiler found. Install MinGW-w64 or set PATH to g++.
    pause
    popd
    endlocal
    exit /b 1
)

echo Using compiler: %GPP%

rem If executable not present, try to build; otherwise just run
if not exist "%BIN%\main.exe" (
    echo.
    echo 🔧 Building project with %GPP%...
    "%GPP%" -std=c++17 -Wall -Wextra -Iinclude -I"%SFML_INCLUDE%" -g main.cpp GiaoDienSFML.cpp data\KiemTraDuLieu.cpp data\QuanLySach.cpp data\ThaoTacFile.cpp data\NhapLieu.cpp utils\ThongBao.cpp utils\NgayThang.cpp utils\XuLyChuoi.cpp -I. -L"%SFML_LIB%" -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network -lgdi32 -luser32 -lole32 -loleaut32 -lcomdlg32 -luuid -o "%BIN%\main.exe"

    if errorlevel 1 (
        echo.
        echo ❌ Build failed. Fix compilation errors and try again.
        pause
        popd
        endlocal
        exit /b 1
    ) else (
        echo ✅ Build finished: %BIN%\main.exe
    )
)

rem Copy SFML DLLs into bin if available so program can run without extra PATH setup
if exist "%SFML_BIN%\*.dll" (
    echo Copying SFML DLLs to bin folder...
    xcopy /Y /Q "%SFML_BIN%\*.dll" "%BIN%\" >nul
)

rem Ensure bin is on PATH so DLLs are found when launching
set "PATH=%BIN%;%PATH%"

rem Start the program (non-blocking) — use start so console stays usable
if exist "%BIN%\main.exe" (
    echo ▶️ Launching %BIN%\main.exe
    start "LibraryManagement" "%BIN%\main.exe"
) else (
    echo ❌ Executable not found: %BIN%\main.exe
    pause
    popd
    endlocal
    exit /b 1
)

rem Done
endlocal
popd
exit /b 0
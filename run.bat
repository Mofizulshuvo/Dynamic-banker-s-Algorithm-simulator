@echo off
setlocal
pushd "%~dp0"

echo ========================================
echo Dynamic Banker's Algorithm Simulator
echo ========================================
echo.

where g++ >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: g++ was not found in PATH.
    echo Install MinGW-w64/MSYS2 or add your compiler bin folder to PATH.
    echo.
    pause
    exit /b 1
)

echo [1/3] Checking for an old server process...
tasklist /FI "IMAGENAME eq server.exe" 2>nul | find /I "server.exe" >nul
if %errorlevel% equ 0 (
    echo An old server.exe is running. Stopping it so the rebuild is not locked...
    taskkill /F /IM server.exe >nul 2>nul
    timeout /t 1 /nobreak >nul
)

echo [2/3] Compiling C++ backend...
g++ -std=c++17 -D_WIN32_WINNT=0x0A00 -DWIN32_LEAN_AND_MEAN -DNOMINMAX backend/main.cpp backend/server.cpp backend/simulation.cpp backend/banker.cpp backend/fault.cpp backend/recovery.cpp -o backend/server.exe -lws2_32 -static
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Compilation failed. The compiler message above is the real cause.
    echo.
    echo Common fixes:
    echo - Close any old server window and run this file again.
    echo - Make sure you are running from the project root folder.
    echo - Make sure MinGW-w64 g++ is installed and available in PATH.
    echo.
    pause
    exit /b 1
)
echo Backend compiled successfully.
echo.

echo [3/3] Starting HTTP server on port 8080...
echo A separate server window will stay open with live logs.
start "Banker Simulator Server" cmd /k ""%CD%\backend\server.exe""

timeout /t 2 /nobreak >nul

echo Opening web interface at http://localhost:8080...
start "" "http://localhost:8080"

echo.
echo ========================================
echo System ready
echo ========================================
echo URL: http://localhost:8080
echo Keep the server window open while using the simulator.
echo Close that server window when you are done.
echo.
pause

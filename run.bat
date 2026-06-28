@echo off
echo ========================================
echo Dynamic Banker's Algorithm Simulator
echo ========================================
echo.

REM Compile C++ backend with HTTP server
echo [1/2] Compiling C++ backend with HTTP server...
g++ -std=c++17 -DWIN32_LEAN_AND_MEAN -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS backend/main.cpp backend/server.cpp backend/simulation.cpp backend/banker.cpp backend/fault.cpp backend/recovery.cpp -o backend/server.exe -lws2_32 -static
if %errorlevel% neq 0 (
    echo ERROR: Compilation failed!
    echo.
    echo Troubleshooting:
    echo 1. Ensure you have g++ with C++17 support
    echo 2. This requires Windows 10 or later (cpp-httplib requirement)
    echo 3. Try using MSVC (Visual Studio) instead
    pause
    exit /b 1
)
echo Backend compiled successfully.
echo.

REM Start HTTP server
echo [2/2] Starting HTTP server on port 8080...
echo Server will serve both API and frontend...
echo.
start backend\server.exe

REM Wait for server to start
timeout /t 2 /nobreak >nul

REM Open browser to the web interface
echo Opening web interface at http://localhost:8080...
start http://localhost:8080

echo.
echo ========================================
echo System ready!
echo - HTTP server running on port 8080
echo - Web interface opened in browser
echo - Press Ctrl+C in server window to stop
echo ========================================
echo.
echo Instructions:
echo 1. Edit matrices in the web interface
echo 2. Click "Initialize" to set up the system
echo 3. Click "Run" to start simulation
echo 4. Use "Inject Fault" to test fault injection
echo 5. Use "Recovery" to apply recovery strategies
echo.
echo The server will continue running until you close its window.
echo.
pause
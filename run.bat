@echo off
echo ========================================
echo Dynamic Banker's Algorithm Simulator
echo ========================================
echo.

REM Compile C++ backend
echo [1/3] Compiling C++ backend...
g++ backend/main.cpp backend/banker.cpp backend/fault.cpp -o backend/app
if %errorlevel% neq 0 (
    echo ERROR: Compilation failed!
    echo Make sure g++ is installed and in PATH.
    pause
    exit /b 1
)
echo Backend compiled successfully.
echo.

REM Run C++ backend
echo [2/3] Running C++ backend...
backend\app
if %errorlevel% neq 0 (
    echo ERROR: Backend execution failed!
    pause
    exit /b 1
)
echo Backend execution completed.
echo.

REM Open frontend in browser
echo [3/3] Opening web interface...
start frontend\index.html

echo.
echo ========================================
echo System ready!
echo - Backend compiled and executed
echo - Web interface opened in browser
echo - Check data/output.json for results
echo ========================================
echo.
echo Instructions:
echo 1. Edit matrices in the web interface
echo 2. Click "Download Input" to save input.json
echo 3. Copy input.json to data/ folder
echo 4. Run this batch file again
echo 5. Click "Upload Output" in web interface
echo.
pause
@echo off
setlocal
cd /d "%~dp0"

REM If g++ is not in user PATH, look in standard MinGW / MSYS2 locations
where g++ >nul 2>&1
if errorlevel 1 (
    if exist "C:\msys64\ucrt64\bin\g++.exe" set "PATH=C:\msys64\ucrt64\bin;%PATH%"
    if exist "C:\MinGW\bin\g++.exe" set "PATH=C:\MinGW\bin;%PATH%"
)

if not exist "bin" mkdir "bin"

if not exist "bin\StockPricePredictor.exe" (
    echo ===================================================
    echo  Compiling ALPHA STOCX PRO [Classy GUI Edition]...
    echo ===================================================
    g++ -std=c++17 -Wall -Wextra -Iinclude src/main.cpp src/CSVHandler.cpp src/LinearRegression.cpp src/MovingAverage.cpp src/Portfolio.cpp src/Stock.cpp src/StockManager.cpp src/StockGUI.cpp -mwindows -lgdiplus -ldwmapi -o bin/StockPricePredictor.exe
)

if exist "bin\StockPricePredictor.exe" (
    echo ===================================================
    echo  Launching ALPHA STOCX PRO GUI...
    echo ===================================================
    start "" "bin\StockPricePredictor.exe"
) else (
    echo [ERROR] Failed to compile or find bin\StockPricePredictor.exe
    pause
)


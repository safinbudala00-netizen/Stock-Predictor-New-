# ALPHA STOCX PRO
### Algorithmic Stock Price Predictor & Portfolio Intelligence Studio

ALPHA STOCX PRO is a high-performance, native desktop financial analysis and algorithmic price forecasting application written in **Modern C++17**. Built with native Win32 API and GDI+ hardware-accelerated double buffering, it delivers a responsive, sleek dark-themed fintech dashboard with **zero external heavyweight runtime dependencies** (no Qt, Electron, or web wrappers required).

---

## Key Features

- **Dashboard & Real-Time Market Overview**
  - High-level KPIs: Active equities count, market average, highest valuation, and accessible entry points.
  - Watchlist table displaying current prices, 10-day historical trends, and dynamic price ranges.
  - Quick action shortcuts to trigger analytics, predictions, or buy orders directly.

- **Interactive Stock Analytics & Technical Charts**
  - High-precision visual price curve rendering with gradient underfill.
  - Dynamic horizontal price grids and historical date mappings.
  - Interactive mouse-tracking tooltips showing exact date, closing price, and day-over-day price delta.
  - Key technical indicators: Day 1 Open, 10-Day High, 10-Day Low, and 10-Day Average.

- **AI Predictive Engine (Forward Day 11 Forecasting)**
  - **Ordinary Least Squares (OLS) Linear Regression**: Calculates trend slope ($m$), intercept ($c$), and statistical confidence ($R^2$).
  - **Simple Moving Average (SMA)**: Configurable multi-session window smoothing.
  - Visual projection chart extending historical prices to a forward forecast target (Day 11) with dotted trajectory lines.

- **Head-to-Head Equity Comparison**
  - Side-by-side performance comparison of any two listed equities.
  - Gain comparison, historical price spread, and comparative valuation summary.

- **Portfolio Tracker & Asset Valuation**
  - Real-time mark-to-market valuation, cumulative invested capital, and unrealized Profit & Loss (P&L).
  - Built-in order execution simulator (add/remove positions with quantity and custom purchase price controls).
  - Persistent data storage saved to CSV.

- **Dual-Mode Architecture**
  - **Classy Desktop GUI Mode**: Default modern dark-themed window with smooth typography (Segoe UI) and custom anti-aliased vector iconography.
  - **Console Mode Fallback**: Run with `--cli` or `--console` for automated workflows, terminal environments, or headless servers.

---

## Project Structure

The project follows a clean, modular structure:

```text
Stock-Predictor/
├── run.bat                                # Root one-click build and launch script
├── .gitignore                             # Git exclusion for binaries & temporary files
├── README.md                              # Project documentation
├── .vscode/
│   └── tasks.json                         # VS Code build task configuration
└── StockPricePredictor/
    ├── run.bat                            # Local build and launch script
    ├── include/                           # C++ Header files (.h)
    │   ├── CSVHandler.h                   # CSV data parser & loader
    │   ├── LinearRegression.h             # OLS regression forecasting model
    │   ├── MovingAverage.h                # Simple Moving Average model
    │   ├── Portfolio.h                    # Holdings, cost basis & P&L calculations
    │   ├── Stock.h                        # Stock equity entity and technical metrics
    │   ├── StockGUI.h                     # Win32 / GDI+ GUI architecture & events
    │   ├── StockManager.h                 # Stock dataset manager
    │   └── StockPredictor.h               # Abstract base prediction interface
    ├── src/                               # C++ Implementation files (.cpp)
    │   ├── CSVHandler.cpp
    │   ├── LinearRegression.cpp
    │   ├── main.cpp                       # Dual WinMain / console entry point
    │   ├── MovingAverage.cpp
    │   ├── Portfolio.cpp
    │   ├── Stock.cpp
    │   ├── StockGUI.cpp                   # High-DPI GDI+ rendering pipeline
    │   └── StockManager.cpp
    ├── data/                              # Persistent datasets
    │   ├── stocks.csv                     # Historical multi-session equity data
    │   └── portfolio.csv                  # User portfolio holdings
    └── bin/                               # Compiled binaries (generated on build)
        └── StockPricePredictor.exe
```

---

## Prerequisites

- **Operating System**: Windows 10 or Windows 11 (64-bit recommended)
- **Compiler**: GCC / MinGW-w64 with **C++17** support (`g++`)
  - Standard MSYS2 / MinGW installation (e.g. `C:\msys64\ucrt64\bin` or `C:\MinGW\bin`).
- **Dependencies**: Native Windows system libraries (pre-installed on all Windows systems):
  - `gdiplus` (GDI+ 2D Graphics Engine)
  - `gdi32` (Windows Graphics Device Interface)
  - `dwmapi` (Desktop Window Manager for Dark Mode Titlebars)

---

## Quick Start & Running

### Option 1: One-Click Launch (Recommended)
Simply double-click **`run.bat`** in the project root.
The script will:
1. Automatically detect your MinGW / GCC compiler.
2. Compile the project if the binary has not yet been built.
3. Launch the desktop GUI immediately.

### Option 2: Visual Studio Code
1. Open the `Stock-Predictor` folder in VS Code.
2. Press **`Ctrl + Shift + B`** to trigger the default build task.
3. Run `bin\StockPricePredictor.exe` or execute `.\run.bat`.

### Option 3: Manual Command Line Build
Open PowerShell or Command Prompt, navigate to the `StockPricePredictor` folder, and compile with `g++`:

```cmd
cd StockPricePredictor
if not exist "bin" mkdir "bin"
g++ -std=c++17 -Wall -Wextra -Iinclude src/main.cpp src/CSVHandler.cpp src/LinearRegression.cpp src/MovingAverage.cpp src/Portfolio.cpp src/Stock.cpp src/StockManager.cpp src/StockGUI.cpp -mwindows -lgdiplus -ldwmapi -o bin/StockPricePredictor.exe
```

To run:
```cmd
bin\StockPricePredictor.exe
```

To launch in CLI terminal mode:
```cmd
bin\StockPricePredictor.exe --cli
```

---

## Mathematical Models

### 1. Simple Moving Average (SMA)
Calculates the unweighted mean of the previous $N$ recorded closing prices:

$$\text{SMA} = \frac{1}{N} \sum_{i=1}^{N} P_i$$

### 2. Ordinary Least Squares (OLS) Linear Regression
Fits a first-order linear trend equation ($y = mx + c$) across historical session indices:

- **Slope ($m$)**:
  $$m = \frac{N \sum (xy) - (\sum x)(\sum y)}{N \sum (x^2) - (\sum x)^2}$$

- **Intercept ($c$)**:
  $$c = \bar{y} - m\bar{x}$$

- **Coefficient of Determination ($R^2$)**:
  $$R^2 = 1 - \frac{\sum (y_i - \hat{y}_i)^2}{\sum (y_i - \bar{y})^2}$$

---

## Contributing & License

Pull requests and feature suggestions are welcome! Feel free to open an issue or submit a PR to expand technical indicators, add multi-day projections, or connect live REST market feeds.

Licensed under the [MIT License](LICENSE).


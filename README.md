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

```text
Stock-Predictor/
├── run.bat                                # Root one-click build and launch script
├── .gitignore                             # Git exclusion for binaries & temporary files
├── README.md                              # Complete step-by-step documentation
├── .vscode/
│   └── tasks.json                         # VS Code build task configuration
└── StockPricePredictor/
    ├── run.bat                            # Subfolder build and launch script
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

## Prerequisites & Environment Setup

Before running the program, ensure your Windows machine meets the following requirements:

### 1. System Requirements
- **OS**: Windows 10 or Windows 11 (64-bit recommended).
- **Libraries**: Native Windows libraries (`gdiplus`, `gdi32`, `dwmapi`) — **no installation required**, these are pre-installed on every modern Windows system.

### 2. C++ Compiler (GCC / MinGW-w64 with C++17)
You need a GCC C++ compiler (`g++`).

#### How to check if you already have `g++`:
Open Command Prompt or PowerShell and run:
```cmd
g++ --version
```
If you see version info (e.g., `g++ (Rev..., Built by MSYS2/MinGW...) 10.x` or higher), **you are ready to proceed to the running steps below**.

#### If `g++` is not installed or not recognized:
Choose one of the quick options below:
- **Option A (Easiest - WinLibs standalone)**:
  1. Download the latest GCC release zip from [WinLibs](https://winlibs.com/) (e.g. *UCRT runtime / POSIX threads / 64-bit*).
  2. Extract it to `C:\MinGW` or `C:\mingw64`.
  3. Add `C:\MinGW\bin` (or your chosen path) to your System Environment Variable `PATH`.
- **Option B (MSYS2)**:
  1. Download and install [MSYS2](https://www.msys2.org/).
  2. In MSYS2 terminal, run: `pacman -S mingw-w64-ucrt-x86_64-gcc`
  3. Add `C:\msys64\ucrt64\bin` to your Windows `PATH`.

---

## Step-by-Step Running Guide

Follow these simple steps to download, compile, and run the program from scratch:

### Step 1: Clone or Download the Repository

#### Via Git (Recommended):
Open your terminal (PowerShell or Command Prompt) and run:
```cmd
git clone https://github.com/safinbudala00-netizen/Stock-Predictor-New-.git
cd Stock-Predictor-New-
```

#### Via ZIP Download:
1. Click the green **Code** button at the top of the GitHub repository page.
2. Select **Download ZIP**.
3. Extract the downloaded ZIP folder to your desired location (e.g., `C:\Projects\Stock-Predictor`).
4. Open the extracted folder.

---

### Step 2: Run the Program

Choose whichever method you prefer:

#### Method 1: One-Click Automated Run (Easiest & Recommended)
1. Open the project root folder in File Explorer.
2. **Double-click `run.bat`**.
3. What happens automatically:
   - The script detects your GCC/MinGW compiler (even if not yet permanently registered in your PATH).
   - Creates the `bin/` directory if missing.
   - Compiles the source files into `StockPricePredictor/bin/StockPricePredictor.exe`.
   - Automatically launches the dark-themed desktop GUI!

---

#### Method 2: Manual Terminal / Command Line Build

If you prefer building and running from PowerShell or Command Prompt:

1. Open PowerShell or Command Prompt.
2. Navigate into the `StockPricePredictor` directory:
   ```cmd
   cd StockPricePredictor
   ```
3. Create the `bin` folder (if it doesn't exist):
   ```cmd
   if not exist "bin" mkdir "bin"
   ```
4. Compile using `g++`:
   ```cmd
   g++ -std=c++17 -Wall -Wextra -Iinclude src/main.cpp src/CSVHandler.cpp src/LinearRegression.cpp src/MovingAverage.cpp src/Portfolio.cpp src/Stock.cpp src/StockManager.cpp src/StockGUI.cpp -mwindows -lgdiplus -ldwmapi -o bin/StockPricePredictor.exe
   ```
5. Launch the application:
   - **For Desktop GUI Mode**:
     ```cmd
     bin\StockPricePredictor.exe
     ```
   - **For Terminal / Console Mode**:
     ```cmd
     bin\StockPricePredictor.exe --cli
     ```

---

#### Method 3: Using Visual Studio Code

1. Open VS Code and select **File > Open Folder...**, then select the project root folder.
2. Install the **C/C++** extension (by Microsoft) if prompted.
3. Press **`Ctrl + Shift + B`** (or go to **Terminal > Run Build Task...**) to compile.
4. Once built, run the executable by:
   - Double-clicking `run.bat`, or
   - Running `.\StockPricePredictor\bin\StockPricePredictor.exe` in the integrated terminal.

---

## Dual-Mode Usage: GUI vs CLI

| Mode | Command | Description |
| :--- | :--- | :--- |
| **Desktop GUI (Default)** | `StockPricePredictor.exe` or `run.bat` | High-DPI interactive dark dashboard with technical charts, tooltips, and portfolio controls. |
| **Terminal / CLI Mode** | `StockPricePredictor.exe --cli` | Interactive terminal interface suitable for headless environments, scripts, or SSH sessions. |

---

## Troubleshooting & FAQ

### 1. `'g++' is not recognized as an internal or external command`
- **Cause**: The compiler is either not installed or its `bin` folder is not added to your Windows environment variables (`PATH`).
- **Fix**:
  1. Press `Win + R`, type `sysdm.cpl`, press Enter, and navigate to **Advanced > Environment Variables**.
  2. Under **User variables** or **System variables**, select `Path` and click **Edit**.
  3. Click **New** and add the path to your MinGW or MSYS2 bin folder (e.g. `C:\msys64\ucrt64\bin` or `C:\MinGW\bin`).
  4. Restart your Command Prompt / terminal and verify with `g++ --version`.
  *Note*: The included `run.bat` already checks common installation paths (`C:\msys64\ucrt64\bin` and `C:\MinGW\bin`) automatically.

### 2. Windows Defender / SmartScreen warns about an unrecognized app
- **Cause**: Because you compiled the `.exe` directly on your machine from C++ source, it does not have a paid commercial code-signing certificate.
- **Fix**: Click **More info** and then click **Run anyway**. The entire source code is 100% open, transparent, and built locally on your machine.

### 3. "No stock data loaded" error
- Ensure you do not delete the `StockPricePredictor/data/stocks.csv` file. The program automatically checks `data/stocks.csv`, `StockPricePredictor/data/stocks.csv`, and sibling paths, but the file must remain present.

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

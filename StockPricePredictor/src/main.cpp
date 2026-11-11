#include <iostream>
#include <memory>
#include <limits>
#include <cctype>
#include <fstream>
#include <windows.h>
#include "CSVHandler.h"
#include "StockManager.h"
#include "MovingAverage.h"
#include "LinearRegression.h"
#include "Portfolio.h"
#include "StockGUI.h"

using namespace std;

int readInt(const string& message) {
    int value;

    while (true) {
        cout << message;

        if (cin >> value)
            return value;

        cout << "Please enter a valid number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

double readDouble(const string& message) {
    double value;

    while (true) {
        cout << message;

        if (cin >> value && value >= 0)
            return value;

        cout << "Please enter a valid non-negative number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

string readSymbol(const string& message) {
    string symbol;

    cout << message;
    cin >> symbol;

    for (char& ch : symbol)
        ch = toupper(static_cast<unsigned char>(ch));

    return symbol;
}

void predictPrice(const StockManager& manager) {
    string symbol = readSymbol("Enter stock symbol: ");

    const Stock* stock = manager.search(symbol);

    if (!stock) {
        cout << "Stock not found.\n";
        return;
    }

    cout << "1. Moving Average\n";
    cout << "2. Linear Regression\n";

    int choice = readInt("Choose prediction method: ");

    unique_ptr<StockPredictor> predictor;

    if (choice == 1)
        predictor = make_unique<MovingAverage>();
    else if (choice == 2)
        predictor = make_unique<LinearRegression>();
    else {
        cout << "Invalid choice.\n";
        return;
    }

    try {
        cout << "Estimated next price for " << symbol
             << ": Rs. "
             << predictor->predict(stock->getPrices())
             << '\n';

        cout << "Note: This is a simple estimate, not a guaranteed market price.\n";
    }
    catch (const exception& e) {
        cout << e.what() << '\n';
    }
}

void portfolioMenu(Portfolio& portfolio,
                   const StockManager& manager) {

    cout << "\n1. Add holding\n";
    cout << "2. View portfolio\n";
    cout << "3. Remove holding\n";

    int choice = readInt("Choose: ");

    if (choice == 1) {
        string symbol = readSymbol("Enter stock symbol: ");

        if (!manager.search(symbol)) {
            cout << "Stock not found in stock data.\n";
            return;
        }

        int quantity = readInt("Enter quantity: ");
        double price = readDouble("Enter buy price: ");

        portfolio.add(symbol, quantity, price);
    }
    else if (choice == 2) {
        portfolio.display();
    }
    else if (choice == 3) {
        portfolio.remove(
            readSymbol("Enter stock symbol to remove: ")
        );
    }
    else {
        cout << "Invalid choice.\n";
    }
}

int runConsole(StockManager& manager, Portfolio& portfolio) {
    int choice;

    do {
        cout << "\n====== STOCK PRICE PREDICTOR ======\n";
        cout << "1. Display all stocks\n";
        cout << "2. Search for a stock\n";
        cout << "3. View historical prices\n";
        cout << "4. Predict stock price\n";
        cout << "5. Compare two stocks\n";
        cout << "6. Manage portfolio\n";
        cout << "7. Market analysis\n";
        cout << "8. Exit\n";

        choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1:
                manager.displayAll();
                break;

            case 2: {
                string symbol = readSymbol("Enter stock symbol: ");
                const Stock* stock = manager.search(symbol);
                if (stock)
                    stock->display();
                else
                    cout << "Stock not found.\n";
                break;
            }

            case 3:
                manager.showHistory(
                    readSymbol("Enter stock symbol: ")
                );
                break;

            case 4:
                predictPrice(manager);
                break;

            case 5: {
                string a = readSymbol("Enter first stock symbol: ");
                string b = readSymbol("Enter second stock symbol: ");
                manager.compare(a, b);
                break;
            }

            case 6:
                portfolioMenu(portfolio, manager);
                break;

            case 7:
                manager.marketAnalysis();
                break;

            case 8:
                cout << "Thank you for using Stock Price Predictor!\n";
                break;

            default:
                cout << "Invalid choice. Please select 1 to 8.\n";
        }

    } while (choice != 8);

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;

    string cmd(lpCmdLine ? lpCmdLine : "");
    bool forceCli = (cmd.find("--cli") != string::npos || cmd.find("--console") != string::npos);

    string dataPath = "data/stocks.csv";
    string portfolioPath = "data/portfolio.csv";

    // Locate stocks.csv across possible working directories
    if (!ifstream(dataPath).good()) {
        if (ifstream("../data/stocks.csv").good()) {
            dataPath = "../data/stocks.csv";
        } else if (ifstream("StockPricePredictor/data/stocks.csv").good()) {
            dataPath = "StockPricePredictor/data/stocks.csv";
        } else if (ifstream("../StockPricePredictor/data/stocks.csv").good()) {
            dataPath = "../StockPricePredictor/data/stocks.csv";
        }
    }

    // Locate portfolio.csv across possible working directories
    if (!ifstream(portfolioPath).good()) {
        if (ifstream("../data/portfolio.csv").good()) {
            portfolioPath = "../data/portfolio.csv";
        } else if (ifstream("StockPricePredictor/data/portfolio.csv").good()) {
            portfolioPath = "StockPricePredictor/data/portfolio.csv";
        } else if (ifstream("../StockPricePredictor/data/portfolio.csv").good()) {
            portfolioPath = "../StockPricePredictor/data/portfolio.csv";
        } else if (ifstream("portfolio.csv").good()) {
            portfolioPath = "portfolio.csv";
        } else if (ifstream("../portfolio.csv").good()) {
            portfolioPath = "../portfolio.csv";
        } else if (ifstream("StockPricePredictor/portfolio.csv").good()) {
            portfolioPath = "StockPricePredictor/portfolio.csv";
        }
    }

    vector<Stock> data = CSVHandler::loadStocks(dataPath);

    if (data.empty()) {
        if (forceCli) {
            cout << "No stock data loaded. Make sure data/stocks.csv exists.\n";
        } else {
            MessageBoxW(NULL,
                L"No stock data could be loaded.\nPlease ensure data/stocks.csv exists in the data directory.",
                L"Data Loading Error",
                MB_ICONERROR | MB_OK);
        }
        return 1;
    }

    StockManager manager(data);
    Portfolio portfolio(portfolioPath);

    if (forceCli) {
        if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) {
            FILE* fp;
            freopen_s(&fp, "CONIN$", "r", stdin);
            freopen_s(&fp, "CONOUT$", "w", stdout);
            freopen_s(&fp, "CONOUT$", "w", stderr);
        }
        return runConsole(manager, portfolio);
    }

    StockGUI gui(manager, portfolio);
    return gui.run(hInstance, nCmdShow);
}

int main(int argc, char* argv[]) {
    string cmdArgs = "";
    for (int i = 1; i < argc; i++) {
        cmdArgs += argv[i];
        cmdArgs += " ";
    }
    return WinMain(GetModuleHandle(NULL), NULL, const_cast<char*>(cmdArgs.c_str()), SW_SHOW);
}

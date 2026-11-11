#include "StockManager.h"
#include <iostream>
#include <iomanip>

using namespace std;

StockManager::StockManager(const vector<Stock>& data)
    : stocks(data) {}

const vector<Stock>& StockManager::getStocks() const {
    return stocks;
}

void StockManager::displayAll() const {

    if (stocks.empty()) {
        cout << "No stock data available.\n";
        return;
    }

    cout << "\n--- AVAILABLE STOCKS ---\n";

    for (const auto& stock : stocks)
        stock.display();
}

const Stock* StockManager::search(const string& symbol) const {

    for (const auto& stock : stocks) {

        if (stock.getSymbol() == symbol)
            return &stock;
    }

    return nullptr;
}

void StockManager::showHistory(const string& symbol) const {

    const Stock* stock = search(symbol);

    if (!stock) {
        cout << "Stock not found.\n";
        return;
    }

    vector<double> prices = stock->getPrices();

    cout << "\nHistorical prices for " << symbol << ":\n";

    for (size_t i = 0; i < prices.size(); ++i) {

        cout << "Day " << i + 1
             << ": Rs. "
             << fixed << setprecision(2)
             << prices[i] << '\n';
    }
}

void StockManager::compare(const string& first,
                           const string& second) const {

    const Stock* a = search(first);
    const Stock* b = search(second);

    if (!a || !b) {
        cout << "One or both stocks were not found.\n";
        return;
    }

    cout << a->getSymbol()
         << " latest price: Rs. "
         << a->getLatestPrice() << '\n';

    cout << b->getSymbol()
         << " latest price: Rs. "
         << b->getLatestPrice() << '\n';

    if (a->getLatestPrice() > b->getLatestPrice())
        cout << first << " has the higher latest price.\n";

    else if (a->getLatestPrice() < b->getLatestPrice())
        cout << second << " has the higher latest price.\n";

    else
        cout << "Both have the same latest price.\n";
}

void StockManager::marketAnalysis() const {

    if (stocks.empty()) {
        cout << "No stock data available.\n";
        return;
    }

    double total = 0;

    const Stock* highest = &stocks[0];
    const Stock* lowest = &stocks[0];

    for (const auto& stock : stocks) {

        double p = stock.getLatestPrice();

        total += p;

        if (p > highest->getLatestPrice())
            highest = &stock;

        if (p < lowest->getLatestPrice())
            lowest = &stock;
    }

    cout << "\n--- BASIC MARKET ANALYSIS ---\n";

    cout << "Number of stocks: "
         << stocks.size() << '\n';

    cout << "Average latest price: Rs. "
         << fixed << setprecision(2)
         << total / stocks.size() << '\n';

    cout << "Highest priced: "
         << highest->getSymbol()
         << " (Rs. " << highest->getLatestPrice() << ")\n";

    cout << "Lowest priced: "
         << lowest->getSymbol()
         << " (Rs. " << lowest->getLatestPrice() << ")\n";
}

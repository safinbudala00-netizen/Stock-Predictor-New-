#include "Portfolio.h"
#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

Portfolio::Portfolio(string file)
    : filename(file) {
    load();
}

void Portfolio::load() {

    holdings.clear();

    ifstream in(filename);

    if (!in)
        return;

    string line;

    getline(in, line);

    while (getline(in, line)) {

        size_t a = line.find(',');

        size_t b = line.find(
            ',',
            a == string::npos ? a : a + 1
        );

        if (a == string::npos || b == string::npos)
            continue;

        try {

            Holding h{
                line.substr(0, a),
                stoi(line.substr(a + 1, b - a - 1)),
                stod(line.substr(b + 1))
            };

            if (h.quantity > 0 && h.buyPrice >= 0)
                holdings.push_back(h);
        }
        catch (...) {
            // Ignore invalid rows.
        }
    }
}

void Portfolio::save() const {

    ofstream out(filename);

    out << "Symbol,Quantity,BuyPrice\n";

    for (const auto& h : holdings) {

        out << h.symbol << ','
            << h.quantity << ','
            << h.buyPrice << '\n';
    }
}

void Portfolio::add(const string& symbol,
                    int quantity,
                    double buyPrice) {

    if (quantity <= 0 || buyPrice < 0) {

        cout << "Enter a positive quantity and a valid price.\n";
        return;
    }

    for (auto& h : holdings) {

        if (h.symbol == symbol) {

            double totalCost =
                h.quantity * h.buyPrice +
                quantity * buyPrice;

            h.quantity += quantity;

            h.buyPrice = totalCost / h.quantity;

            save();

            cout << "Holding updated.\n";
            return;
        }
    }

    holdings.push_back({symbol, quantity, buyPrice});

    save();

    cout << "Stock added to portfolio.\n";
}

void Portfolio::remove(const string& symbol) {

    for (auto it = holdings.begin();
         it != holdings.end(); ++it) {

        if (it->symbol == symbol) {

            holdings.erase(it);

            save();

            cout << "Holding removed.\n";
            return;
        }
    }

    cout << "Holding not found.\n";
}

void Portfolio::display() const {

    if (holdings.empty()) {

        cout << "Portfolio is empty.\n";
        return;
    }

    cout << "\n--- YOUR PORTFOLIO ---\n";

    cout << left
         << setw(15) << "Symbol"
         << setw(12) << "Quantity"
         << "Average buy price\n";

    for (const auto& h : holdings) {

        cout << left
             << setw(15) << h.symbol
             << setw(12) << h.quantity
             << fixed << setprecision(2)
             << h.buyPrice << '\n';
    }
}

const vector<Holding>& Portfolio::getHoldings() const {
    return holdings;
}

const string& Portfolio::getFilename() const {
    return filename;
}

const Holding* Portfolio::getHolding(const string& symbol) const {
    for (const auto& h : holdings) {
        if (h.symbol == symbol) {
            return &h;
        }
    }
    return nullptr;
}

double Portfolio::getTotalCost() const {
    double total = 0.0;
    for (const auto& h : holdings) {
        total += h.quantity * h.buyPrice;
    }
    return total;
}

double Portfolio::getTotalMarketValue(const StockManager& manager) const {
    double total = 0.0;
    for (const auto& h : holdings) {
        const Stock* s = manager.search(h.symbol);
        double price = s ? s->getLatestPrice() : h.buyPrice;
        total += h.quantity * price;
    }
    return total;
}

double Portfolio::getTotalProfitLoss(const StockManager& manager) const {
    return getTotalMarketValue(manager) - getTotalCost();
}

double Portfolio::getTotalProfitLossPercent(const StockManager& manager) const {
    double cost = getTotalCost();
    if (cost <= 0.0) return 0.0;
    return (getTotalProfitLoss(manager) / cost) * 100.0;
}


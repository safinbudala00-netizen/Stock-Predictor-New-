#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include <string>
#include <vector>
#include "StockManager.h"

using namespace std;

struct Holding {
    string symbol;
    int quantity;
    double buyPrice;
};

class Portfolio {

private:
    vector<Holding> holdings;
    string filename;

    void save() const;

public:
    explicit Portfolio(string file = "portfolio.csv");

    void load();

    void add(const string& symbol,
             int quantity,
             double buyPrice);

    void remove(const string& symbol);

    void display() const;

    const vector<Holding>& getHoldings() const;
    const string& getFilename() const;

    const Holding* getHolding(const string& symbol) const;
    double getTotalCost() const;
    double getTotalMarketValue(const StockManager& manager) const;
    double getTotalProfitLoss(const StockManager& manager) const;
    double getTotalProfitLossPercent(const StockManager& manager) const;
};

#endif

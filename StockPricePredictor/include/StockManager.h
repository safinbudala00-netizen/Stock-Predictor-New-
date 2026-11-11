#ifndef STOCKMANAGER_H
#define STOCKMANAGER_H

#include <vector>
#include <string>
#include "Stock.h"

using namespace std;

class StockManager {

private:
    vector<Stock> stocks;

public:
    explicit StockManager(const vector<Stock>& data);

    const vector<Stock>& getStocks() const;

    void displayAll() const;

    const Stock* search(const string& symbol) const;

    void showHistory(const string& symbol) const;

    void compare(const string& first,
                 const string& second) const;

    void marketAnalysis() const;
};

#endif

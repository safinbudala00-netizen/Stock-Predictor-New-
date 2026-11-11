#ifndef STOCK_H
#define STOCK_H

#include <string>
#include <vector>

using namespace std;

class Stock {

private:
    string symbol, company;
    vector<double> prices;
    vector<string> dates;

public:
    Stock();

    Stock(string s, string c);

    void addPrice(double price);
    void addPrice(double price, const string& date);

    string getSymbol() const;
    string getCompany() const;

    vector<double> getPrices() const;
    const vector<string>& getDates() const;

    double getLatestPrice() const;
    double getFirstPrice() const;
    double getPriceChange() const;
    double getPriceChangePercent() const;
    double getMinPrice() const;
    double getMaxPrice() const;
    double getAveragePrice() const;

    void display() const;
};

#endif

#include "Stock.h"
#include <iostream>
#include <numeric>
#include <algorithm>
#include <iomanip>

using namespace std;

Stock::Stock() : symbol(""), company("") {}

Stock::Stock(string s, string c)
    : symbol(s), company(c) {}

void Stock::addPrice(double price) {
    prices.push_back(price);
}

void Stock::addPrice(double price, const string& date) {
    prices.push_back(price);
    if (!date.empty()) {
        dates.push_back(date);
    }
}

string Stock::getSymbol() const {
    return symbol;
}

string Stock::getCompany() const {
    return company;
}

vector<double> Stock::getPrices() const {
    return prices;
}

const vector<string>& Stock::getDates() const {
    return dates;
}

double Stock::getLatestPrice() const {
    return prices.empty() ? 0.0 : prices.back();
}

double Stock::getFirstPrice() const {
    return prices.empty() ? 0.0 : prices.front();
}

double Stock::getPriceChange() const {
    if (prices.size() < 2) return 0.0;
    return prices.back() - prices.front();
}

double Stock::getPriceChangePercent() const {
    if (prices.size() < 2 || prices.front() == 0.0) return 0.0;
    return (getPriceChange() / prices.front()) * 100.0;
}

double Stock::getMinPrice() const {
    if (prices.empty()) return 0.0;
    return *min_element(prices.begin(), prices.end());
}

double Stock::getMaxPrice() const {
    if (prices.empty()) return 0.0;
    return *max_element(prices.begin(), prices.end());
}

double Stock::getAveragePrice() const {
    if (prices.empty()) return 0.0;
    double sum = accumulate(prices.begin(), prices.end(), 0.0);
    return sum / prices.size();
}

void Stock::display() const {
    cout << symbol << " - " << company
         << " | Latest price: Rs. "
         << fixed << setprecision(2) << getLatestPrice() << '\n';
}

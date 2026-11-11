#include "CSVHandler.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

vector<Stock> CSVHandler::loadStocks(
    const string& filename
) {

    vector<Stock> stocks;

    ifstream file(filename);

    if (!file) {

        cerr << "Could not open "
             << filename << '\n';

        return stocks;
    }

    string line, symbol, company, date, priceText;

    getline(file, line); // Skip header

    while (getline(file, line)) {

        stringstream ss(line);

        getline(ss, symbol, ',');
        getline(ss, company, ',');
        getline(ss, date, ',');
        getline(ss, priceText, ',');

        if (symbol.empty() || priceText.empty())
            continue;

        try {

            double price = stod(priceText);

            size_t i = 0;

            for (; i < stocks.size(); ++i) {

                if (stocks[i].getSymbol() == symbol)
                    break;
            }

            if (i == stocks.size())
                stocks.emplace_back(symbol, company);

            stocks[i].addPrice(price, date);
        }
        catch (...) {

            cerr << "Skipping invalid CSV row: "
                 << line << '\n';
        }
    }

    return stocks;
}

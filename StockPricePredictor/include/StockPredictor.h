#ifndef STOCKPREDICTOR_H
#define STOCKPREDICTOR_H

#include <vector>

using namespace std;

class StockPredictor {

public:
    virtual double predict(
        const vector<double>& prices
    ) const = 0;

    virtual ~StockPredictor() = default;
};

#endif

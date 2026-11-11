#ifndef MOVINGAVERAGE_H
#define MOVINGAVERAGE_H

#include "StockPredictor.h"

struct MovingAverageDetail {
    int window = 5;
    double sum = 0.0;
    double average = 0.0;
    vector<double> windowPrices;
};

class MovingAverage : public StockPredictor {

public:
    double predict(
        const vector<double>& prices
    ) const override;

    MovingAverageDetail calculateDetail(
        const vector<double>& prices,
        int window = 5
    ) const;
};

#endif

#ifndef LINEARREGRESSION_H
#define LINEARREGRESSION_H

#include "StockPredictor.h"

struct RegressionDetail {
    double slope = 0.0;
    double intercept = 0.0;
    double nextPrice = 0.0;
    double rSquared = 0.0;
};

class LinearRegression : public StockPredictor {

public:
    double predict(
        const vector<double>& prices
    ) const override;

    RegressionDetail calculateDetail(
        const vector<double>& prices
    ) const;
};

#endif

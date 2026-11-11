#include "LinearRegression.h"
#include <stdexcept>

RegressionDetail LinearRegression::calculateDetail(
    const vector<double>& prices
) const {
    RegressionDetail detail;
    int n = static_cast<int>(prices.size());

    if (n < 2)
        throw runtime_error("At least two prices are required.");

    double sumX = 0;
    double sumY = 0;
    double sumXY = 0;
    double sumX2 = 0;

    for (int i = 0; i < n; i++) {
        double x = i + 1;
        double y = prices[i];

        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }

    double denominator = n * sumX2 - sumX * sumX;

    if (denominator == 0)
        throw runtime_error("Cannot calculate regression.");

    detail.slope = (n * sumXY - sumX * sumY) / denominator;
    detail.intercept = (sumY - detail.slope * sumX) / n;
    detail.nextPrice = detail.intercept + detail.slope * (n + 1);

    // Calculate R-squared
    double meanY = sumY / n;
    double ssTot = 0.0;
    double ssRes = 0.0;
    for (int i = 0; i < n; i++) {
        double y = prices[i];
        double yPred = detail.intercept + detail.slope * (i + 1);
        ssTot += (y - meanY) * (y - meanY);
        ssRes += (y - yPred) * (y - yPred);
    }
    detail.rSquared = (ssTot > 0.0) ? (1.0 - (ssRes / ssTot)) : 1.0;
    if (detail.rSquared < 0.0) detail.rSquared = 0.0;

    return detail;
}

double LinearRegression::predict(
    const vector<double>& prices
) const {
    return calculateDetail(prices).nextPrice;
}

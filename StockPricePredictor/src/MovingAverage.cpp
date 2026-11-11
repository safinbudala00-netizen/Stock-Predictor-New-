#include "MovingAverage.h"
#include <stdexcept>

MovingAverageDetail MovingAverage::calculateDetail(
    const vector<double>& prices,
    int window
) const {
    if (prices.empty())
        throw runtime_error("No price history available.");

    int count = prices.size() < static_cast<size_t>(window)
                ? static_cast<int>(prices.size())
                : window;

    MovingAverageDetail detail;
    detail.window = count;
    detail.sum = 0.0;

    int startIdx = static_cast<int>(prices.size()) - count;
    for (int i = startIdx; i < static_cast<int>(prices.size()); i++) {
        detail.sum += prices[i];
        detail.windowPrices.push_back(prices[i]);
    }

    detail.average = detail.sum / count;
    return detail;
}

double MovingAverage::predict(
    const vector<double>& prices
) const {
    return calculateDetail(prices, 5).average;
}

#ifndef CSVHANDLER_H
#define CSVHANDLER_H

#include <string>
#include <vector>
#include "Stock.h"

using namespace std;

class CSVHandler {

public:
    static vector<Stock> loadStocks(
        const string& filename
    );
};

#endif

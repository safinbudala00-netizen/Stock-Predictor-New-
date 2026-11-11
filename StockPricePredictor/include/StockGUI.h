#ifndef STOCKGUI_H
#define STOCKGUI_H

#include <windows.h>
#include <string>
#include <vector>
#include <memory>
#include "StockManager.h"
#include "Portfolio.h"
#include "MovingAverage.h"
#include "LinearRegression.h"

enum NavTab {
    TAB_OVERVIEW = 0,
    TAB_CHARTS = 1,
    TAB_PREDICTOR = 2,
    TAB_COMPARE = 3,
    TAB_PORTFOLIO = 4
};

enum PredictModel {
    MODEL_MOVING_AVERAGE = 0,
    MODEL_LINEAR_REGRESSION = 1
};

class StockGUI {
public:
    StockGUI(StockManager& manager, Portfolio& portfolio);
    ~StockGUI();

    int run(HINSTANCE hInstance, int nCmdShow);

private:
    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void onPaint(HWND hwnd);
    void onMouseMove(int x, int y);
    void onMouseDown(int x, int y);
    void onMouseUp(int x, int y);
    void onKeyDown(WPARAM key);
    void onChar(wchar_t ch);
    void onResize(int width, int height);

    void showToast(const std::string& message, bool isError = false);
    void updateFilteredStocks();

    // Data references
    StockManager& stockManager;
    Portfolio& portfolio;

    MovingAverage movingAveragePredictor;
    LinearRegression linearRegressionPredictor;

    // Window state
    HWND hWnd;
    int clientWidth;
    int clientHeight;
    HBITMAP hMemBmp;
    HDC hMemDC;
    int bmpWidth;
    int bmpHeight;

    // UI state
    NavTab currentTab;
    int selectedChartStock;
    int selectedPredictStock;
    PredictModel currentModel;
    int compareStockA;
    int compareStockB;

    // Portfolio input state
    int portfolioStockIndex;
    int portfolioQuantity;
    double portfolioBuyPrice;

    // Search state
    std::string searchQuery;
    bool searchFocused;
    std::vector<int> filteredIndices;

    // Chart hover state
    int hoverChartPoint;
    float hoverMouseX;
    float hoverMouseY;
    bool isMouseInChart;

    // Interactive button state
    int hoveredButtonId;

    // Toast notification
    std::string toastMessage;
    DWORD toastTimestamp;
    bool toastError;

    // Caret blink for search box
    bool caretVisible;
};

#endif


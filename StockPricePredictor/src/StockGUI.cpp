#define UNICODE
#define _UNICODE
#include "StockGUI.h"
#include <gdiplus.h>
#include <dwmapi.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

using namespace Gdiplus;

// --- Helper Functions for GDI+ Rendering ---

static std::wstring toWide(const std::string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    if (size <= 1) {
        return std::wstring(str.begin(), str.end());
    }
    std::wstring wstr(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size);
    return wstr;
}

static std::wstring formatCurrency(double val) {
    std::wstringstream ss;
    ss << L"Rs. " << std::fixed << std::setprecision(2) << val;
    return ss.str();
}

static std::wstring formatPercent(double val, bool includeSign = true) {
    std::wstringstream ss;
    if (includeSign && val > 0) ss << L"+";
    ss << std::fixed << std::setprecision(2) << val << L"%";
    return ss.str();
}

static void drawRoundedRect(Graphics& g, Pen* pen, Brush* brush, float x, float y, float w, float h, float r) {
    if (w <= 0.0f || h <= 0.0f) return;
    if (r <= 0.0f) {
        if (brush) g.FillRectangle(brush, x, y, w, h);
        if (pen) g.DrawRectangle(pen, x, y, w, h);
        return;
    }
    if (r * 2.0f > w) r = w / 2.0f;
    if (r * 2.0f > h) r = h / 2.0f;

    GraphicsPath path;
    path.AddArc(x, y, r * 2.0f, r * 2.0f, 180.0f, 90.0f);
    path.AddArc(x + w - r * 2.0f, y, r * 2.0f, r * 2.0f, 270.0f, 90.0f);
    path.AddArc(x + w - r * 2.0f, y + h - r * 2.0f, r * 2.0f, r * 2.0f, 0.0f, 90.0f);
    path.AddArc(x, y + h - r * 2.0f, r * 2.0f, r * 2.0f, 90.0f, 90.0f);
    path.CloseFigure();

    if (brush) g.FillPath(brush, &path);
    if (pen) g.DrawPath(pen, &path);
}

// Vector Icon Drawing Helper (Eliminates broken emoji square boxes completely)
static void drawNavVectorIcon(Graphics& g, NavTab tab, float x, float y, Color col) {
    Pen pen(col, 2.0f);
    pen.SetLineJoin(LineJoinRound);
    pen.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
    SolidBrush brush(col);

    switch (tab) {
        case TAB_OVERVIEW: {
            g.FillRectangle(&brush, x, y + 8.0f, 3.5f, 8.0f);
            g.FillRectangle(&brush, x + 5.5f, y + 2.0f, 3.5f, 14.0f);
            g.FillRectangle(&brush, x + 11.0f, y + 5.0f, 3.5f, 11.0f);
            break;
        }
        case TAB_CHARTS: {
            PointF pts[4] = { {x, y + 13.0f}, {x + 5.0f, y + 8.0f}, {x + 10.0f, y + 11.0f}, {x + 15.0f, y + 2.0f} };
            g.DrawLines(&pen, pts, 4);
            g.FillEllipse(&brush, x + 13.5f, y + 0.5f, 4.0f, 4.0f);
            break;
        }
        case TAB_PREDICTOR: {
            GraphicsPath spark;
            spark.AddLine(x + 7.5f, y, x + 9.5f, y + 5.5f);
            spark.AddLine(x + 9.5f, y + 5.5f, x + 15.0f, y + 7.5f);
            spark.AddLine(x + 15.0f, y + 7.5f, x + 9.5f, y + 9.5f);
            spark.AddLine(x + 9.5f, y + 9.5f, x + 7.5f, y + 15.0f);
            spark.AddLine(x + 7.5f, y + 15.0f, x + 5.5f, y + 9.5f);
            spark.AddLine(x + 5.5f, y + 9.5f, x, y + 7.5f);
            spark.AddLine(x, y + 7.5f, x + 5.5f, y + 5.5f);
            spark.CloseFigure();
            g.FillPath(&brush, &spark);
            break;
        }
        case TAB_COMPARE: {
            g.DrawLine(&pen, x, y + 4.5f, x + 13.0f, y + 4.5f);
            g.DrawLine(&pen, x + 9.0f, y + 1.5f, x + 13.0f, y + 4.5f);
            g.DrawLine(&pen, x + 9.0f, y + 7.5f, x + 13.0f, y + 4.5f);

            g.DrawLine(&pen, x + 15.0f, y + 11.5f, x + 2.0f, y + 11.5f);
            g.DrawLine(&pen, x + 6.0f, y + 8.5f, x + 2.0f, y + 11.5f);
            g.DrawLine(&pen, x + 6.0f, y + 14.5f, x + 2.0f, y + 11.5f);
            break;
        }
        case TAB_PORTFOLIO: {
            drawRoundedRect(g, &pen, NULL, x, y + 4.0f, 15.0f, 11.0f, 2.0f);
            g.DrawArc(&pen, x + 4.5f, y + 0.5f, 6.0f, 5.0f, 180.0f, 180.0f);
            g.FillRectangle(&brush, x + 6.5f, y + 8.5f, 2.0f, 2.0f);
            break;
        }
    }
}

// UI Interaction IDs
enum ButtonId {
    BTN_NONE = 0,
    BTN_TAB_OVERVIEW = 101,
    BTN_TAB_CHARTS = 102,
    BTN_TAB_PREDICTOR = 103,
    BTN_TAB_COMPARE = 104,
    BTN_TAB_PORTFOLIO = 105,
    BTN_SEARCH_BAR = 150,

    BTN_MODEL_SMA = 201,
    BTN_MODEL_LR = 202,

    BTN_PORTFOLIO_ADD = 301,
    BTN_QTY_M10 = 310,
    BTN_QTY_M1 = 311,
    BTN_QTY_P1 = 312,
    BTN_QTY_P10 = 313,
    BTN_PRICE_M10 = 320,
    BTN_PRICE_M1 = 321,
    BTN_PRICE_P1 = 322,
    BTN_PRICE_P10 = 323,

    BTN_STOCK_BASE = 1000,
    BTN_STOCK_COMPARE_A_BASE = 2000,
    BTN_STOCK_COMPARE_B_BASE = 3000,
    BTN_STOCK_PORT_BASE = 4000,
    BTN_REMOVE_HOLDING_BASE = 5000,
    BTN_DASH_CHART_BASE = 6000,
    BTN_DASH_PRED_BASE = 7000,
    BTN_DASH_BUY_BASE = 8000
};

struct ClickableRegion {
    RectF rect;
    int id;
    std::string text;
};

static std::vector<ClickableRegion> g_clickables;

StockGUI::StockGUI(StockManager& manager, Portfolio& p)
    : stockManager(manager),
      portfolio(p),
      hWnd(NULL),
      clientWidth(1360),
      clientHeight(840),
      hMemBmp(NULL),
      hMemDC(NULL),
      bmpWidth(0),
      bmpHeight(0),
      currentTab(TAB_OVERVIEW),
      selectedChartStock(0),
      selectedPredictStock(0),
      currentModel(MODEL_LINEAR_REGRESSION),
      compareStockA(0),
      compareStockB(1),
      portfolioStockIndex(0),
      portfolioQuantity(10),
      portfolioBuyPrice(0.0),
      searchQuery(""),
      searchFocused(false),
      hoverChartPoint(-1),
      hoverMouseX(0),
      hoverMouseY(0),
      isMouseInChart(false),
      hoveredButtonId(BTN_NONE),
      toastMessage(""),
      toastTimestamp(0),
      toastError(false),
      caretVisible(true)
{
    const auto& stocks = stockManager.getStocks();
    if (!stocks.empty()) {
        portfolioBuyPrice = stocks[0].getLatestPrice();
        if (stocks.size() > 1) {
            compareStockB = 1;
        }
    }
    updateFilteredStocks();
}

StockGUI::~StockGUI() {
    if (hMemBmp) DeleteObject(hMemBmp);
    if (hMemDC) DeleteDC(hMemDC);
}

void StockGUI::updateFilteredStocks() {
    filteredIndices.clear();
    const auto& stocks = stockManager.getStocks();
    if (searchQuery.empty()) {
        for (int i = 0; i < (int)stocks.size(); i++) {
            filteredIndices.push_back(i);
        }
        return;
    }

    std::string q = searchQuery;
    std::transform(q.begin(), q.end(), q.begin(), ::toupper);

    for (int i = 0; i < (int)stocks.size(); i++) {
        std::string sym = stocks[i].getSymbol();
        std::string comp = stocks[i].getCompany();
        std::transform(sym.begin(), sym.end(), sym.begin(), ::toupper);
        std::transform(comp.begin(), comp.end(), comp.begin(), ::toupper);

        if (sym.find(q) != std::string::npos || comp.find(q) != std::string::npos) {
            filteredIndices.push_back(i);
        }
    }
}

void StockGUI::showToast(const std::string& message, bool isError) {
    toastMessage = message;
    toastTimestamp = GetTickCount();
    toastError = isError;
    if (hWnd) InvalidateRect(hWnd, NULL, FALSE);
}

int StockGUI::run(HINSTANCE hInstance, int nCmdShow) {
    (void)nCmdShow;

    // Enable DPI awareness
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        typedef BOOL(WINAPI * SetProcessDPIAwareFunc)();
        SetProcessDPIAwareFunc setDpi = reinterpret_cast<SetProcessDPIAwareFunc>(
            reinterpret_cast<void*>(GetProcAddress(hUser32, "SetProcessDPIAware"))
        );
        if (setDpi) setDpi();
    }

    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WNDCLASSEXW wc;
    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = StaticWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = L"StockPredictorGUIClass";

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"Window Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    // Large & Spacious Default Dimensions
    int initW = 1360;
    int initH = 840;
    if (initW > screenW - 60) initW = screenW - 60;
    if (initH > screenH - 60) initH = screenH - 60;

    int posX = (screenW - initW) / 2;
    int posY = (screenH - initH) / 2;

    hWnd = CreateWindowExW(
        WS_EX_APPWINDOW,
        L"StockPredictorGUIClass",
        L"ALPHA STOCX PRO • Predictive Financial Intelligence Studio",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        posX, posY, initW, initH,
        NULL, NULL, hInstance, this
    );

    if (!hWnd) {
        MessageBoxW(NULL, L"Window Creation Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Set immersive dark mode for title bar
    BOOL darkMode = TRUE;
    DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));

    // Timer for caret blinking and toast fade
    SetTimer(hWnd, 1, 500, NULL);

    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);
    SetForegroundWindow(hWnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    KillTimer(hWnd, 1);
    GdiplusShutdown(gdiplusToken);
    return (int)msg.wParam;
}

LRESULT CALLBACK StockGUI::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    StockGUI* pThis = nullptr;
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (StockGUI*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->hWnd = hwnd;
    } else {
        pThis = (StockGUI*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (pThis) {
        return pThis->WndProc(hwnd, msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT StockGUI::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    try {
        switch (msg) {
            case WM_GETMINMAXINFO: {
                LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
                lpMMI->ptMinTrackSize.x = 1120;
                lpMMI->ptMinTrackSize.y = 720;
                return 0;
            }

            case WM_SIZE: {
                int w = LOWORD(lParam);
                int h = HIWORD(lParam);
                onResize(w, h);
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }

            case WM_ERASEBKGND:
                return 1; // Prevent flicker

            case WM_PAINT:
                onPaint(hwnd);
                return 0;

            case WM_MOUSEMOVE: {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                onMouseMove(x, y);

                TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, hwnd, 0 };
                TrackMouseEvent(&tme);
                return 0;
            }

            case WM_MOUSELEAVE:
                isMouseInChart = false;
                hoverChartPoint = -1;
                hoveredButtonId = BTN_NONE;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;

            case WM_LBUTTONDOWN: {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                onMouseDown(x, y);
                return 0;
            }

            case WM_LBUTTONUP: {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                onMouseUp(x, y);
                return 0;
            }

            case WM_KEYDOWN:
                onKeyDown(wParam);
                return 0;

            case WM_CHAR:
                onChar((wchar_t)wParam);
                return 0;

            case WM_TIMER:
                caretVisible = !caretVisible;
                if (searchFocused || (toastMessage.length() > 0 && GetTickCount() - toastTimestamp > 4000)) {
                    if (toastMessage.length() > 0 && GetTickCount() - toastTimestamp > 4000) {
                        toastMessage = "";
                    }
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                return 0;

            case WM_SETCURSOR: {
                if (hoveredButtonId != BTN_NONE || isMouseInChart) {
                    SetCursor(LoadCursor(NULL, IDC_HAND));
                    return TRUE;
                }
                if (searchFocused) {
                    POINT pt;
                    GetCursorPos(&pt);
                    ScreenToClient(hwnd, &pt);
                    if (pt.x >= clientWidth - 320 && pt.x <= clientWidth - 24 && pt.y >= 15 && pt.y <= 50) {
                        SetCursor(LoadCursor(NULL, IDC_IBEAM));
                        return TRUE;
                    }
                }
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                return TRUE;
            }

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
    } catch (...) {}
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void StockGUI::onResize(int width, int height) {
    clientWidth = width;
    clientHeight = height;

    if (hMemBmp) {
        DeleteObject(hMemBmp);
        hMemBmp = NULL;
    }
    if (hMemDC) {
        DeleteDC(hMemDC);
        hMemDC = NULL;
    }
    bmpWidth = 0;
    bmpHeight = 0;
}

void StockGUI::onMouseMove(int x, int y) {
    hoverMouseX = (float)x;
    hoverMouseY = (float)y;

    int oldHovered = hoveredButtonId;
    hoveredButtonId = BTN_NONE;

    for (const auto& btn : g_clickables) {
        if (btn.rect.Contains((float)x, (float)y)) {
            hoveredButtonId = btn.id;
            break;
        }
    }

    if (oldHovered != hoveredButtonId || isMouseInChart) {
        InvalidateRect(hWnd, NULL, FALSE);
    }
}

void StockGUI::onMouseDown(int x, int y) {
    float searchX = (float)clientWidth - 280.0f - 24.0f;
    if (x >= searchX && x <= searchX + 280.0f && y >= 15 && y <= 50) {
        searchFocused = true;
    } else {
        searchFocused = false;
    }

    for (const auto& btn : g_clickables) {
        if (btn.rect.Contains((float)x, (float)y)) {
            int id = btn.id;

            // Nav Tabs
            if (id == BTN_TAB_OVERVIEW) { currentTab = TAB_OVERVIEW; }
            else if (id == BTN_TAB_CHARTS) { currentTab = TAB_CHARTS; }
            else if (id == BTN_TAB_PREDICTOR) { currentTab = TAB_PREDICTOR; }
            else if (id == BTN_TAB_COMPARE) { currentTab = TAB_COMPARE; }
            else if (id == BTN_TAB_PORTFOLIO) { currentTab = TAB_PORTFOLIO; }

            // Model Selection
            else if (id == BTN_MODEL_SMA) {
                currentModel = MODEL_MOVING_AVERAGE;
                showToast("Prediction Model: 5-Day Simple Moving Average (SMA)");
            }
            else if (id == BTN_MODEL_LR) {
                currentModel = MODEL_LINEAR_REGRESSION;
                showToast("Prediction Model: Linear Regression (OLS Trend)");
            }

            // Portfolio Stock Selection
            else if (id >= BTN_STOCK_PORT_BASE && id < BTN_STOCK_PORT_BASE + 500) {
                portfolioStockIndex = id - BTN_STOCK_PORT_BASE;
                const auto& stocks = stockManager.getStocks();
                if (portfolioStockIndex >= 0 && portfolioStockIndex < (int)stocks.size()) {
                    portfolioBuyPrice = stocks[portfolioStockIndex].getLatestPrice();
                }
            }

            // Chart Stock Selection
            else if (id >= BTN_STOCK_BASE && id < BTN_STOCK_BASE + 500) {
                if (currentTab == TAB_CHARTS) {
                    selectedChartStock = id - BTN_STOCK_BASE;
                } else if (currentTab == TAB_PREDICTOR) {
                    selectedPredictStock = id - BTN_STOCK_BASE;
                }
            }

            // Compare Stock Pickers
            else if (id >= BTN_STOCK_COMPARE_A_BASE && id < BTN_STOCK_COMPARE_A_BASE + 500) {
                compareStockA = id - BTN_STOCK_COMPARE_A_BASE;
            }
            else if (id >= BTN_STOCK_COMPARE_B_BASE && id < BTN_STOCK_COMPARE_B_BASE + 500) {
                compareStockB = id - BTN_STOCK_COMPARE_B_BASE;
            }

            // Quantity Buttons
            else if (id == BTN_QTY_M10) { if (portfolioQuantity > 10) portfolioQuantity -= 10; }
            else if (id == BTN_QTY_M1) { if (portfolioQuantity > 1) portfolioQuantity -= 1; }
            else if (id == BTN_QTY_P1) { portfolioQuantity += 1; }
            else if (id == BTN_QTY_P10) { portfolioQuantity += 10; }

            // Price Buttons
            else if (id == BTN_PRICE_M10) { if (portfolioBuyPrice > 10) portfolioBuyPrice -= 10; }
            else if (id == BTN_PRICE_M1) { if (portfolioBuyPrice > 1) portfolioBuyPrice -= 1; }
            else if (id == BTN_PRICE_P1) { portfolioBuyPrice += 1; }
            else if (id == BTN_PRICE_P10) { portfolioBuyPrice += 10; }

            // Add to Portfolio
            else if (id == BTN_PORTFOLIO_ADD) {
                const auto& stocks = stockManager.getStocks();
                if (portfolioStockIndex >= 0 && portfolioStockIndex < (int)stocks.size()) {
                    std::string sym = stocks[portfolioStockIndex].getSymbol();
                    portfolio.add(sym, portfolioQuantity, portfolioBuyPrice);
                    showToast("Successfully added " + std::to_string(portfolioQuantity) + " shares of " + sym + " to portfolio!");
                }
            }

            // Remove Holding
            else if (id >= BTN_REMOVE_HOLDING_BASE && id < BTN_REMOVE_HOLDING_BASE + 500) {
                int holdIdx = id - BTN_REMOVE_HOLDING_BASE;
                const auto& holdings = portfolio.getHoldings();
                if (holdIdx >= 0 && holdIdx < (int)holdings.size()) {
                    std::string sym = holdings[holdIdx].symbol;
                    portfolio.remove(sym);
                    showToast("Removed holding: " + sym);
                }
            }

            // Dashboard Quick Actions
            else if (id >= BTN_DASH_CHART_BASE && id < BTN_DASH_CHART_BASE + 500) {
                selectedChartStock = id - BTN_DASH_CHART_BASE;
                currentTab = TAB_CHARTS;
            }
            else if (id >= BTN_DASH_PRED_BASE && id < BTN_DASH_PRED_BASE + 500) {
                selectedPredictStock = id - BTN_DASH_PRED_BASE;
                currentTab = TAB_PREDICTOR;
            }
            else if (id >= BTN_DASH_BUY_BASE && id < BTN_DASH_BUY_BASE + 500) {
                portfolioStockIndex = id - BTN_DASH_BUY_BASE;
                const auto& stocks = stockManager.getStocks();
                if (portfolioStockIndex >= 0 && portfolioStockIndex < (int)stocks.size()) {
                    portfolioBuyPrice = stocks[portfolioStockIndex].getLatestPrice();
                }
                currentTab = TAB_PORTFOLIO;
            }

            InvalidateRect(hWnd, NULL, FALSE);
            break;
        }
    }
}

void StockGUI::onMouseUp(int, int) {}

void StockGUI::onKeyDown(WPARAM key) {
    if (searchFocused) {
        if (key == VK_ESCAPE) {
            searchQuery.clear();
            searchFocused = false;
            updateFilteredStocks();
            InvalidateRect(hWnd, NULL, FALSE);
        } else if (key == VK_BACK && !searchQuery.empty()) {
            searchQuery.pop_back();
            updateFilteredStocks();
            InvalidateRect(hWnd, NULL, FALSE);
        }
    }
}

void StockGUI::onChar(wchar_t ch) {
    if (searchFocused) {
        if (ch >= 32 && ch < 127) {
            searchQuery.push_back((char)ch);
            updateFilteredStocks();
            InvalidateRect(hWnd, NULL, FALSE);
        }
    }
}

// --- MAIN ONPAINT METHOD ---

void StockGUI::onPaint(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    RECT rc;
    GetClientRect(hwnd, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    if (width <= 0 || height <= 0) {
        EndPaint(hwnd, &ps);
        return;
    }

    // Prepare double-buffering
    if (!hMemDC) {
        hMemDC = CreateCompatibleDC(hdc);
    }
    if (!hMemBmp || bmpWidth != width || bmpHeight != height) {
        if (hMemBmp) DeleteObject(hMemBmp);
        hMemBmp = CreateCompatibleBitmap(hdc, width, height);
        SelectObject(hMemDC, hMemBmp);
        bmpWidth = width;
        bmpHeight = height;
    }

    g_clickables.clear();

    try {
        Graphics g(hMemDC);
        g.SetSmoothingMode(SmoothingModeAntiAlias);
        g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

        // Theme Palette
        Color colBg(255, 11, 15, 25);            // Deep dark slate
        Color colSidebar(255, 15, 23, 42);       // Dark navy sidebar
        Color colHeader(255, 13, 19, 33);        // Dark topbar
        Color colCard(255, 22, 32, 50);          // Card background
        Color colCardHover(255, 28, 42, 66);     // Card highlight
        Color colBorder(255, 34, 49, 71);        // Card border
        Color colBorderLight(255, 45, 63, 90);   // Accent border
        Color colAccent(255, 56, 189, 248);      // Cyan Azure #38BDF8
        Color colGreen(255, 16, 185, 129);       // Emerald Green #10B981
        Color colGreenBg(40, 16, 185, 129);
        Color colRed(255, 244, 63, 94);          // Rose Red #F43F5E
        Color colRedBg(40, 244, 63, 94);
        Color colPurple(255, 139, 92, 246);      // Violet #8B5CF6
        Color colAmber(255, 245, 158, 11);       // Warm Amber #F59E0B
        Color colTextWhite(255, 248, 250, 252);  // Crisp white
        Color colTextSecondary(255, 148, 163, 184); // Slate muted
        Color colTextMuted(255, 100, 116, 139);  // Faint label

        // Brushes and Pens
        SolidBrush bBg(colBg);
        SolidBrush bSidebar(colSidebar);
        SolidBrush bHeader(colHeader);
        SolidBrush bCard(colCard);
        SolidBrush bCardHover(colCardHover);
        SolidBrush bTextWhite(colTextWhite);
        SolidBrush bTextSecondary(colTextSecondary);
        SolidBrush bTextMuted(colTextMuted);
        SolidBrush bAccent(colAccent);
        SolidBrush bGreen(colGreen);
        SolidBrush bRed(colRed);
        SolidBrush bPurple(colPurple);
        SolidBrush bAmber(colAmber);

        Pen pBorder(colBorder, 1.0f);
        Pen pBorderLight(colBorderLight, 1.0f);
        Pen pAccent(colAccent, 2.0f);
        Pen pGreen(colGreen, 2.0f);
        Pen pRed(colRed, 2.0f);
        Pen pPurple(colPurple, 2.0f);
        Pen pAmber(colAmber, 2.0f);

        // Fonts
        FontFamily ff(L"Segoe UI");
        FontFamily ffMono(L"Consolas");

        Font fLogo(&ff, 15, FontStyleBold, UnitPoint);
        Font fSubLogo(&ff, 8.5f, FontStyleRegular, UnitPoint);
        Font fPageTitle(&ff, 15, FontStyleBold, UnitPoint);
        Font fSectionTitle(&ff, 12.0f, FontStyleBold, UnitPoint);
        Font fNav(&ff, 9.5f, FontStyleRegular, UnitPoint);
        Font fNavBold(&ff, 9.5f, FontStyleBold, UnitPoint);
        Font fHeroPrice(&ff, 21, FontStyleBold, UnitPoint);
        Font fLargeNum(&ff, 16, FontStyleBold, UnitPoint);
        Font fMediumNum(&ff, 13, FontStyleBold, UnitPoint);
        Font fBody(&ff, 9.0f, FontStyleRegular, UnitPoint);
        Font fBodyBold(&ff, 9.0f, FontStyleBold, UnitPoint);
        Font fSmall(&ff, 8.0f, FontStyleRegular, UnitPoint);
        Font fSmallBold(&ff, 8.0f, FontStyleBold, UnitPoint);
        Font fFormula(&ffMono, 9.5f, FontStyleRegular, UnitPoint);

        StringFormat sfCenter;
        sfCenter.SetAlignment(StringAlignmentCenter);
        sfCenter.SetLineAlignment(StringAlignmentCenter);

        StringFormat sfLeft;
        sfLeft.SetAlignment(StringAlignmentNear);
        sfLeft.SetLineAlignment(StringAlignmentCenter);

        StringFormat sfRight;
        sfRight.SetAlignment(StringAlignmentFar);
        sfRight.SetLineAlignment(StringAlignmentCenter);

        // 1. Fill Background
        g.FillRectangle(&bBg, 0, 0, width, height);

        // 2. Draw Left Sidebar
        float sideW = 230.0f;
        g.FillRectangle(&bSidebar, 0.0f, 0.0f, sideW, (float)height);
        g.DrawLine(&pBorder, sideW, 0.0f, sideW, (float)height);

        // Sidebar Brand Logo
        drawRoundedRect(g, NULL, &bAccent, 20.0f, 18.0f, 34.0f, 34.0f, 7.0f);
        g.DrawString(L"▲", -1, &fSectionTitle, PointF(27.0f, 22.0f), &sfLeft, &bCard);
        g.DrawString(L"STOCX PRO", -1, &fLogo, PointF(64.0f, 18.0f), &sfLeft, &bTextWhite);
        g.DrawString(L"Predictive Intelligence Studio", -1, &fSubLogo, PointF(64.0f, 37.0f), &sfLeft, &bTextSecondary);

        g.DrawLine(&pBorder, 15.0f, 68.0f, sideW - 15.0f, 68.0f);

        // Sidebar Navigation Items with Clean Vector Icons
        struct NavItem {
            NavTab tab;
            int btnId;
            const wchar_t* label;
        };
        NavItem navItems[] = {
            { TAB_OVERVIEW,  BTN_TAB_OVERVIEW,  L"Market Overview" },
            { TAB_CHARTS,    BTN_TAB_CHARTS,    L"Stock Analytics" },
            { TAB_PREDICTOR, BTN_TAB_PREDICTOR, L"AI Price Predictor" },
            { TAB_COMPARE,   BTN_TAB_COMPARE,   L"Stock Compare" },
            { TAB_PORTFOLIO, BTN_TAB_PORTFOLIO, L"My Portfolio" }
        };

        float navY = 82.0f;
        for (const auto& item : navItems) {
            bool active = (currentTab == item.tab);
            bool hovered = (hoveredButtonId == item.btnId);

            RectF navRect(12.0f, navY, sideW - 24.0f, 42.0f);
            g_clickables.push_back({ navRect, item.btnId, "" });

            if (active) {
                SolidBrush bActiveBg(Color(45, 56, 189, 248));
                drawRoundedRect(g, &pAccent, &bActiveBg, navRect.X, navRect.Y, navRect.Width, navRect.Height, 7.0f);
                drawRoundedRect(g, NULL, &bAccent, navRect.X + 3.0f, navRect.Y + 7.0f, 3.5f, navRect.Height - 14.0f, 2.0f);
            } else if (hovered) {
                drawRoundedRect(g, &pBorderLight, &bCardHover, navRect.X, navRect.Y, navRect.Width, navRect.Height, 7.0f);
            }

            // Draw Vector Icon
            drawNavVectorIcon(g, item.tab, navRect.X + 16.0f, navRect.Y + 13.0f, active ? colAccent : colTextSecondary);

            // Draw Nav Label
            PointF labelPt(navRect.X + 42.0f, navRect.Y + 21.0f);
            g.DrawString(item.label, -1, active ? &fNavBold : &fNav, labelPt, &sfLeft, active ? &bTextWhite : &bTextSecondary);

            navY += 48.0f;
        }

        // Sidebar Footer Status Card
        float footCardY = (float)height - 105.0f;
        if (footCardY > navY + 30.0f) {
            RectF footRect(14.0f, footCardY, sideW - 28.0f, 85.0f);
            drawRoundedRect(g, &pBorder, &bCard, footRect.X, footRect.Y, footRect.Width, footRect.Height, 8.0f);

            drawRoundedRect(g, NULL, &bGreen, footRect.X + 14.0f, footRect.Y + 16.0f, 7.0f, 7.0f, 3.5f);
            g.DrawString(L"MARKET DATA ACTIVE", -1, &fSmallBold, PointF(footRect.X + 26.0f, footRect.Y + 20.0f), &sfLeft, &bGreen);

            std::wstring dataInfo = std::to_wstring(stockManager.getStocks().size()) + L" Equities Loaded";
            g.DrawString(dataInfo.c_str(), -1, &fSmall, PointF(footRect.X + 14.0f, footRect.Y + 42.0f), &sfLeft, &bTextSecondary);

            std::wstring portInfo = L"Portfolio: " + std::to_wstring(portfolio.getHoldings().size()) + L" holdings saved";
            g.DrawString(portInfo.c_str(), -1, &fSmall, PointF(footRect.X + 14.0f, footRect.Y + 63.0f), &sfLeft, &bTextMuted);
        }

        // 3. Draw Top Header Bar
        float headerH = 60.0f;
        g.FillRectangle(&bHeader, sideW, 0.0f, (float)width - sideW, headerH);
        g.DrawLine(&pBorder, sideW, headerH, (float)width, headerH);

        // Header Title / Breadcrumbs
        const wchar_t* tabTitles[] = {
            L"Dashboard • Real-Time Market Overview",
            L"Stock Deep Dive • Price Chart & Technicals",
            L"AI Predictive Engine • Forward Price Forecast",
            L"Comparative Analysis • Head-to-Head Performance",
            L"Portfolio Tracker • Live Asset Valuation & Risk"
        };
        g.DrawString(tabTitles[currentTab], -1, &fPageTitle, PointF(sideW + 20.0f, headerH / 2.0f), &sfLeft, &bTextWhite);

        // Search Input Box (Right side of header)
        float searchW = 250.0f;
        float searchH = 34.0f;
        float searchX = (float)width - searchW - 20.0f;
        float searchY = (headerH - searchH) / 2.0f;

        RectF searchRect(searchX, searchY, searchW, searchH);
        g_clickables.push_back({ searchRect, BTN_SEARCH_BAR, "" });

        SolidBrush bSearchBg(searchFocused ? Color(255, 26, 38, 58) : Color(255, 20, 29, 44));
        drawRoundedRect(g, searchFocused ? &pAccent : &pBorder, &bSearchBg, searchX, searchY, searchW, searchH, 6.0f);

        // Vector Search Icon
        {
            Pen pSrch(colTextSecondary, 1.5f);
            float icX = searchX + 12.0f;
            float icY = searchY + 11.0f;
            g.DrawEllipse(&pSrch, icX, icY, 8.0f, 8.0f);
            g.DrawLine(&pSrch, icX + 6.0f, icY + 6.0f, icX + 11.0f, icY + 11.0f);
        }

        // Search Text
        if (searchQuery.empty()) {
            g.DrawString(L"Filter stocks...", -1, &fSmall, PointF(searchX + 30.0f, searchY + searchH / 2.0f), &sfLeft, &bTextMuted);
        } else {
            std::wstring wQuery = toWide(searchQuery);
            if (searchFocused && caretVisible) wQuery += L"|";
            g.DrawString(wQuery.c_str(), -1, &fBodyBold, PointF(searchX + 30.0f, searchY + searchH / 2.0f), &sfLeft, &bTextWhite);
        }

        // Content Area Coordinates
        float contX = sideW + 20.0f;
        float contY = headerH + 16.0f;
        float contW = (float)width - sideW - 40.0f;
        float contH = (float)height - headerH - 24.0f;

        const auto& stocks = stockManager.getStocks();

        // 4. Render Active Tab Content
        if (currentTab == TAB_OVERVIEW) {
            // === TAB 1: MARKET OVERVIEW DASHBOARD ===

            // 4 Top Metric Cards (Generous, Spacious Dimensions)
            float cardSpacing = 12.0f;
            float cardW = (contW - (cardSpacing * 3.0f)) / 4.0f;
            float cardH = 95.0f;

            // Compute Market Stats
            double totalMarketPrice = 0.0;
            const Stock* highestStock = stocks.empty() ? nullptr : &stocks[0];
            const Stock* lowestStock = stocks.empty() ? nullptr : &stocks[0];

            for (const auto& s : stocks) {
                double p = s.getLatestPrice();
                totalMarketPrice += p;
                if (highestStock && p > highestStock->getLatestPrice()) highestStock = &s;
                if (lowestStock && p < lowestStock->getLatestPrice()) lowestStock = &s;
            }
            double avgPrice = stocks.empty() ? 0.0 : (totalMarketPrice / stocks.size());

            // Metric 1: Tracked Equities
            RectF m1(contX, contY, cardW, cardH);
            drawRoundedRect(g, &pBorder, &bCard, m1.X, m1.Y, m1.Width, m1.Height, 8.0f);
            g.DrawString(L"TRACKED EQUITIES", -1, &fSmallBold, PointF(m1.X + 16.0f, m1.Y + 18.0f), &sfLeft, &bTextSecondary);
            std::wstring m1Val = std::to_wstring(stocks.size()) + L" Equities";
            g.DrawString(m1Val.c_str(), -1, &fLargeNum, PointF(m1.X + 16.0f, m1.Y + 44.0f), &sfLeft, &bAccent);
            g.DrawString(L"National Stock Exchange", -1, &fSmall, PointF(m1.X + 16.0f, m1.Y + 70.0f), &sfLeft, &bTextMuted);

            // Metric 2: Average Price
            RectF m2(contX + (cardW + cardSpacing), contY, cardW, cardH);
            drawRoundedRect(g, &pBorder, &bCard, m2.X, m2.Y, m2.Width, m2.Height, 8.0f);
            g.DrawString(L"MARKET AVERAGE", -1, &fSmallBold, PointF(m2.X + 16.0f, m2.Y + 18.0f), &sfLeft, &bTextSecondary);
            g.DrawString(formatCurrency(avgPrice).c_str(), -1, &fLargeNum, PointF(m2.X + 16.0f, m2.Y + 44.0f), &sfLeft, &bTextWhite);
            g.DrawString(L"Across all active equities", -1, &fSmall, PointF(m2.X + 16.0f, m2.Y + 70.0f), &sfLeft, &bTextMuted);

            // Metric 3: Highest Priced Stock
            RectF m3(contX + (cardW + cardSpacing) * 2.0f, contY, cardW, cardH);
            drawRoundedRect(g, &pBorder, &bCard, m3.X, m3.Y, m3.Width, m3.Height, 8.0f);
            g.DrawString(L"HIGHEST VALUATION", -1, &fSmallBold, PointF(m3.X + 16.0f, m3.Y + 18.0f), &sfLeft, &bTextSecondary);
            if (highestStock) {
                g.DrawString(toWide(highestStock->getSymbol()).c_str(), -1, &fLargeNum, PointF(m3.X + 16.0f, m3.Y + 44.0f), &sfLeft, &bGreen);
                std::wstring hSub = formatCurrency(highestStock->getLatestPrice()) + L"  •  " + toWide(highestStock->getCompany());
                g.DrawString(hSub.c_str(), -1, &fSmallBold, PointF(m3.X + 16.0f, m3.Y + 70.0f), &sfLeft, &bTextSecondary);
            }

            // Metric 4: Lowest Priced Stock
            RectF m4(contX + (cardW + cardSpacing) * 3.0f, contY, cardW, cardH);
            drawRoundedRect(g, &pBorder, &bCard, m4.X, m4.Y, m4.Width, m4.Height, 8.0f);
            g.DrawString(L"ACCESSIBLE ENTRY", -1, &fSmallBold, PointF(m4.X + 16.0f, m4.Y + 18.0f), &sfLeft, &bTextSecondary);
            if (lowestStock) {
                g.DrawString(toWide(lowestStock->getSymbol()).c_str(), -1, &fLargeNum, PointF(m4.X + 16.0f, m4.Y + 44.0f), &sfLeft, &bAmber);
                std::wstring lSub = formatCurrency(lowestStock->getLatestPrice()) + L"  •  " + toWide(lowestStock->getCompany());
                g.DrawString(lSub.c_str(), -1, &fSmallBold, PointF(m4.X + 16.0f, m4.Y + 70.0f), &sfLeft, &bTextSecondary);
            }

            // Table of Stocks
            float tableY = contY + cardH + 16.0f;
            float tableH = contH - (tableY - contY);

            RectF tableCard(contX, tableY, contW, tableH);
            drawRoundedRect(g, &pBorder, &bCard, tableCard.X, tableCard.Y, tableCard.Width, tableCard.Height, 8.0f);

            // Table Header Bar
            float thY = tableY + 12.0f;
            g.DrawString(L"EQUITY WATCHLIST & REAL-TIME SUMMARY", -1, &fSectionTitle, PointF(tableCard.X + 20.0f, thY + 8.0f), &sfLeft, &bTextWhite);

            // PROPORTIONAL COLUMN COORDINATES WITH RIGHT-ANCHORED ACTIONS (Zero Collision!)
            float padX = 20.0f;
            float tInnerW = contW - (padX * 2.0f);

            float colSymbol  = tableCard.X + padX;
            float colCompany = colSymbol  + tInnerW * 0.11f;
            float colPrice   = colCompany + tInnerW * 0.25f;
            float colChange  = colPrice   + tInnerW * 0.14f;
            float colRange   = colChange  + tInnerW * 0.13f;
            // Actions Cluster Right-Anchored against right edge of card!
            float colActions = tableCard.X + tableCard.Width - padX - 170.0f;

            float thRowY = thY + 34.0f;
            g.DrawLine(&pBorder, tableCard.X + 15.0f, thRowY, tableCard.X + contW - 15.0f, thRowY);

            thRowY += 12.0f;
            g.DrawString(L"SYMBOL", -1, &fSmallBold, PointF(colSymbol, thRowY), &sfLeft, &bTextSecondary);
            g.DrawString(L"COMPANY NAME", -1, &fSmallBold, PointF(colCompany, thRowY), &sfLeft, &bTextSecondary);
            g.DrawString(L"LATEST PRICE", -1, &fSmallBold, PointF(colPrice, thRowY), &sfLeft, &bTextSecondary);
            g.DrawString(L"10-DAY TREND", -1, &fSmallBold, PointF(colChange, thRowY), &sfLeft, &bTextSecondary);
            g.DrawString(L"10-DAY RANGE", -1, &fSmallBold, PointF(colRange, thRowY), &sfLeft, &bTextSecondary);
            g.DrawString(L"ACTIONS", -1, &fSmallBold, PointF(colActions, thRowY), &sfLeft, &bTextSecondary);

            float rowY = thRowY + 20.0f;
            g.DrawLine(&pBorder, tableCard.X + 15.0f, rowY, tableCard.X + contW - 15.0f, rowY);

            rowY += 6.0f;
            for (int idx : filteredIndices) {
                if (idx < 0 || idx >= (int)stocks.size()) continue;
                const Stock& s = stocks[idx];

                RectF rowRect(tableCard.X + 10.0f, rowY, contW - 20.0f, 48.0f);
                bool rowHovered = (hoverMouseX >= rowRect.X && hoverMouseX <= rowRect.X + rowRect.Width &&
                                   hoverMouseY >= rowRect.Y && hoverMouseY <= rowRect.Y + rowRect.Height);

                if (rowHovered) {
                    drawRoundedRect(g, &pBorderLight, &bCardHover, rowRect.X, rowRect.Y, rowRect.Width, rowRect.Height, 6.0f);
                }

                float midY = rowY + 24.0f;

                // Symbol Badge
                RectF symBadge(colSymbol, midY - 11.0f, 75.0f, 22.0f);
                SolidBrush bSymBg(Color(40, 56, 189, 248));
                drawRoundedRect(g, &pAccent, &bSymBg, symBadge.X, symBadge.Y, symBadge.Width, symBadge.Height, 4.0f);
                g.DrawString(toWide(s.getSymbol()).c_str(), -1, &fSmallBold, PointF(symBadge.X + symBadge.Width / 2.0f, midY), &sfCenter, &bAccent);

                // Company Name
                g.DrawString(toWide(s.getCompany()).c_str(), -1, &fBodyBold, PointF(colCompany, midY), &sfLeft, &bTextWhite);

                // Latest Price
                g.DrawString(formatCurrency(s.getLatestPrice()).c_str(), -1, &fBodyBold, PointF(colPrice, midY), &sfLeft, &bTextWhite);

                // Change Badge
                double chg = s.getPriceChange();
                double pct = s.getPriceChangePercent();
                bool positive = (chg >= 0);

                RectF chgBadge(colChange, midY - 11.0f, 96.0f, 22.0f);
                SolidBrush bChgBg(positive ? colGreenBg : colRedBg);
                Pen* pChg = positive ? &pGreen : &pRed;
                drawRoundedRect(g, pChg, &bChgBg, chgBadge.X, chgBadge.Y, chgBadge.Width, chgBadge.Height, 4.0f);

                std::wstring chgText = (positive ? L"▲ +" : L"▼ ") + formatPercent(pct, false);
                g.DrawString(chgText.c_str(), -1, &fSmallBold, PointF(chgBadge.X + chgBadge.Width / 2.0f, midY), &sfCenter, positive ? &bGreen : &bRed);

                // Clean Concise Range String (Zero collision with actions!)
                std::wstringstream rSs;
                rSs << L"Rs. " << (long long)s.getMinPrice() << L" - " << (long long)s.getMaxPrice();
                std::wstring rangeText = rSs.str();
                g.DrawString(rangeText.c_str(), -1, &fSmallBold, PointF(colRange, midY), &sfLeft, &bTextSecondary);

                // Compact, Spacious Action Buttons: [Chart], [Predict], [+ Buy]
                float btnW = 50.0f;
                float btnH = 25.0f;
                float bY = midY - 12.5f;

                // Chart button
                RectF bChart(colActions, bY, btnW, btnH);
                int bChartId = BTN_DASH_CHART_BASE + idx;
                g_clickables.push_back({ bChart, bChartId, "" });
                bool chHover = (hoveredButtonId == bChartId);
                SolidBrush bChBg(chHover ? Color(255, 38, 56, 82) : Color(255, 24, 35, 54));
                drawRoundedRect(g, &pBorderLight, &bChBg, bChart.X, bChart.Y, bChart.Width, bChart.Height, 4.0f);
                g.DrawString(L"Chart", -1, &fSmallBold, PointF(bChart.X + btnW / 2.0f, midY), &sfCenter, &bAccent);

                // Predict button
                RectF bPred(colActions + btnW + 6.0f, bY, btnW, btnH);
                int bPredId = BTN_DASH_PRED_BASE + idx;
                g_clickables.push_back({ bPred, bPredId, "" });
                bool prHover = (hoveredButtonId == bPredId);
                SolidBrush bPrBg(prHover ? Color(255, 50, 35, 75) : Color(255, 32, 24, 52));
                drawRoundedRect(g, &pPurple, &bPrBg, bPred.X, bPred.Y, bPred.Width, bPred.Height, 4.0f);
                g.DrawString(L"Predict", -1, &fSmallBold, PointF(bPred.X + btnW / 2.0f, midY), &sfCenter, &bPurple);

                // Buy button
                RectF bBuy(colActions + (btnW + 6.0f) * 2.0f, bY, btnW, btnH);
                int bBuyId = BTN_DASH_BUY_BASE + idx;
                g_clickables.push_back({ bBuy, bBuyId, "" });
                bool buyHover = (hoveredButtonId == bBuyId);
                SolidBrush bBuyBg(buyHover ? Color(255, 20, 60, 45) : Color(255, 15, 42, 32));
                drawRoundedRect(g, &pGreen, &bBuyBg, bBuy.X, bBuy.Y, bBuy.Width, bBuy.Height, 4.0f);
                g.DrawString(L"+ Buy", -1, &fSmallBold, PointF(bBuy.X + btnW / 2.0f, midY), &sfCenter, &bGreen);

                rowY += 50.0f;
            }
        }
        else if (currentTab == TAB_CHARTS) {
            // === TAB 2: INTERACTIVE STOCK ANALYTICS & CHARTS ===

            // Stock Selector Pill Bar
            float pillX = contX;
            float pillY = contY;
            float pillH = 34.0f;

            g.DrawString(L"SELECT EQUITY:", -1, &fSmallBold, PointF(pillX, pillY + pillH / 2.0f), &sfLeft, &bTextSecondary);
            pillX += 115.0f;

            for (int i = 0; i < (int)stocks.size(); i++) {
                bool sel = (i == selectedChartStock);
                int btnId = BTN_STOCK_BASE + i;
                float pW = 90.0f;
                RectF pRect(pillX, pillY, pW, pillH);
                g_clickables.push_back({ pRect, btnId, "" });

                bool hov = (hoveredButtonId == btnId);
                SolidBrush bPillBg(sel ? Color(255, 56, 189, 248) : (hov ? colCardHover : colCard));
                drawRoundedRect(g, sel ? &pAccent : &pBorder, &bPillBg, pRect.X, pRect.Y, pRect.Width, pRect.Height, 6.0f);

                g.DrawString(toWide(stocks[i].getSymbol()).c_str(), -1, sel ? &fBodyBold : &fBody,
                             PointF(pRect.X + pW / 2.0f, pRect.Y + pillH / 2.0f), &sfCenter, sel ? &bCard : &bTextWhite);

                pillX += pW + 10.0f;
            }

            if (selectedChartStock >= 0 && selectedChartStock < (int)stocks.size()) {
                const Stock& s = stocks[selectedChartStock];
                const auto& prices = s.getPrices();
                const auto& dates = s.getDates();

                // Stock Info Header Card
                float heroY = pillY + pillH + 16.0f;
                float heroH = 76.0f;
                RectF heroCard(contX, heroY, contW, heroH);
                drawRoundedRect(g, &pBorder, &bCard, heroCard.X, heroCard.Y, heroCard.Width, heroCard.Height, 8.0f);

                // Company and Symbol
                g.DrawString(toWide(s.getCompany()).c_str(), -1, &fSectionTitle, PointF(heroCard.X + 20.0f, heroCard.Y + 24.0f), &sfLeft, &bTextWhite);
                std::wstring symTag = L"TICKER: " + toWide(s.getSymbol()) + L" • 10 TRADING SESSIONS";
                g.DrawString(symTag.c_str(), -1, &fSmall, PointF(heroCard.X + 20.0f, heroCard.Y + 48.0f), &sfLeft, &bTextSecondary);

                // Price and Change (Right side of hero)
                float priceRx = heroCard.X + heroCard.Width - 20.0f;
                g.DrawString(formatCurrency(s.getLatestPrice()).c_str(), -1, &fHeroPrice, PointF(priceRx, heroCard.Y + 28.0f), &sfRight, &bTextWhite);

                double chg = s.getPriceChange();
                double pct = s.getPriceChangePercent();
                bool pos = (chg >= 0);
                std::wstring statText = (pos ? L"▲ +" : L"▼ ") + formatCurrency(std::abs(chg)) + L" (" + formatPercent(pct, false) + L")";
                g.DrawString(statText.c_str(), -1, &fBodyBold, PointF(priceRx, heroCard.Y + 54.0f), &sfRight, pos ? &bGreen : &bRed);

                // Main Interactive Chart Canvas (Taller & Spacious)
                float chartY = heroY + heroH + 16.0f;
                float chartH = 370.0f;
                RectF chartCard(contX, chartY, contW, chartH);
                drawRoundedRect(g, &pBorder, &bCard, chartCard.X, chartCard.Y, chartCard.Width, chartCard.Height, 8.0f);

                // Chart Boundaries inside Card
                float padL = 75.0f;
                float padR = 50.0f;
                float padT = 32.0f;
                float padB = 42.0f;

                float cAreaX = chartCard.X + padL;
                float cAreaY = chartCard.Y + padT;
                float cAreaW = chartCard.Width - padL - padR;
                float cAreaH = chartCard.Height - padT - padB;

                if (prices.size() >= 2) {
                    double minP = s.getMinPrice();
                    double maxP = s.getMaxPrice();
                    double range = maxP - minP;
                    if (range <= 0.0) range = 1.0;

                    minP -= range * 0.1;
                    maxP += range * 0.1;
                    range = maxP - minP;

                    // 1. Draw Horizontal Grid Lines and Y-Axis Labels
                    int numGridLines = 5;
                    Pen pGrid(Color(255, 26, 38, 56), 1.0f);
                    pGrid.SetDashStyle(DashStyleDash);

                    for (int k = 0; k <= numGridLines; k++) {
                        float gy = cAreaY + cAreaH * (float)k / (float)numGridLines;
                        double priceAtG = maxP - (range * (double)k / (double)numGridLines);

                        g.DrawLine(&pGrid, cAreaX, gy, cAreaX + cAreaW, gy);
                        g.DrawString(formatCurrency(priceAtG).c_str(), -1, &fSmall, PointF(chartCard.X + padL - 10.0f, gy), &sfRight, &bTextSecondary);
                    }

                    // 2. Generate Chart Points
                    int n = (int)prices.size();
                    std::vector<PointF> chartPoints;
                    for (int i = 0; i < n; i++) {
                        float px = cAreaX + (cAreaW * (float)i / (float)(n - 1));
                        float py = cAreaY + cAreaH - (float)(((prices[i] - minP) / range) * cAreaH);
                        chartPoints.push_back(PointF(px, py));
                    }

                    // 3. Draw Gradient Filled Area
                    GraphicsPath areaPath;
                    areaPath.AddLine(PointF(chartPoints[0].X, cAreaY + cAreaH), chartPoints[0]);
                    for (size_t i = 1; i < chartPoints.size(); i++) {
                        areaPath.AddLine(chartPoints[i - 1], chartPoints[i]);
                    }
                    areaPath.AddLine(chartPoints.back(), PointF(chartPoints.back().X, cAreaY + cAreaH));
                    areaPath.CloseFigure();

                    LinearGradientBrush gradBrush(
                        PointF(0, cAreaY),
                        PointF(0, cAreaY + cAreaH),
                        Color(80, 56, 189, 248),
                        Color(0, 56, 189, 248)
                    );
                    g.FillPath(&gradBrush, &areaPath);

                    // 4. Draw Main Glowing Price Curve
                    Pen pChartLine(colAccent, 2.8f);
                    pChartLine.SetLineJoin(LineJoinRound);
                    for (size_t i = 1; i < chartPoints.size(); i++) {
                        g.DrawLine(&pChartLine, chartPoints[i - 1], chartPoints[i]);
                    }

                    // 5. Draw X-Axis Labels and Points
                    int closestPoint = -1;
                    float minDistance = 99999.0f;

                    for (int i = 0; i < n; i++) {
                        float px = chartPoints[i].X;
                        float py = chartPoints[i].Y;

                        std::wstring dayLbl = L"Day " + std::to_wstring(i + 1);
                        if (i < (int)dates.size() && !dates[i].empty()) {
                            std::string dt = dates[i];
                            if (dt.length() >= 5) dayLbl = toWide(dt.substr(5));
                        }
                        g.DrawString(dayLbl.c_str(), -1, &fSmall, PointF(px, cAreaY + cAreaH + 16.0f), &sfCenter, &bTextSecondary);

                        SolidBrush bNode(colBg);
                        drawRoundedRect(g, &pAccent, &bNode, px - 4.0f, py - 4.0f, 8.0f, 8.0f, 4.0f);

                        float dist = std::abs(hoverMouseX - px);
                        if (dist < minDistance && hoverMouseY >= cAreaY - 20.0f && hoverMouseY <= cAreaY + cAreaH + 20.0f) {
                            minDistance = dist;
                            closestPoint = i;
                        }
                    }

                    // 6. Interactive Tooltip on Mouse Hover
                    if (closestPoint >= 0 && minDistance < 50.0f) {
                        isMouseInChart = true;
                        float px = chartPoints[closestPoint].X;
                        float py = chartPoints[closestPoint].Y;

                        Pen pGuide(Color(180, 56, 189, 248), 1.0f);
                        pGuide.SetDashStyle(DashStyleDash);
                        g.DrawLine(&pGuide, px, cAreaY, px, cAreaY + cAreaH);

                        SolidBrush bGlow(colGreen);
                        drawRoundedRect(g, &pGreen, &bGlow, px - 6.0f, py - 6.0f, 12.0f, 12.0f, 6.0f);

                        float tipW = 165.0f;
                        float tipH = 64.0f;
                        float tipX = px - tipW / 2.0f;
                        if (tipX < cAreaX) tipX = cAreaX;
                        if (tipX + tipW > cAreaX + cAreaW) tipX = cAreaX + cAreaW - tipW;
                        float tipY = py - tipH - 12.0f;
                        if (tipY < cAreaY) tipY = py + 16.0f;

                        RectF tipRect(tipX, tipY, tipW, tipH);
                        SolidBrush bTipBg(Color(240, 16, 24, 38));
                        drawRoundedRect(g, &pAccent, &bTipBg, tipRect.X, tipRect.Y, tipRect.Width, tipRect.Height, 6.0f);

                        std::wstring tipDay = L"Day " + std::to_wstring(closestPoint + 1);
                        if (closestPoint < (int)dates.size() && !dates[closestPoint].empty()) {
                            tipDay += L" (" + toWide(dates[closestPoint]) + L")";
                        }
                        g.DrawString(tipDay.c_str(), -1, &fSmallBold, PointF(tipX + 12.0f, tipY + 16.0f), &sfLeft, &bTextSecondary);
                        g.DrawString(formatCurrency(prices[closestPoint]).c_str(), -1, &fBodyBold, PointF(tipX + 12.0f, tipY + 36.0f), &sfLeft, &bAccent);

                        if (closestPoint > 0) {
                            double prevP = prices[closestPoint - 1];
                            double delta = prices[closestPoint] - prevP;
                            std::wstring dText = (delta >= 0 ? L"+" : L"") + formatCurrency(delta);
                            g.DrawString(dText.c_str(), -1, &fSmallBold, PointF(tipX + tipW - 12.0f, tipY + 36.0f), &sfRight, delta >= 0 ? &bGreen : &bRed);
                        }
                    } else {
                        isMouseInChart = false;
                    }
                }

                // Summary Stats Strip (Below Chart)
                float statStripY = chartY + chartH + 16.0f;
                float statStripH = 62.0f;
                RectF statCard(contX, statStripY, contW, statStripH);
                drawRoundedRect(g, &pBorder, &bCard, statCard.X, statCard.Y, statCard.Width, statCard.Height, 8.0f);

                float sColW = contW / 4.0f;
                g.DrawString(L"DAY 1 OPEN", -1, &fSmallBold, PointF(statCard.X + 20.0f, statCard.Y + 18.0f), &sfLeft, &bTextSecondary);
                g.DrawString(formatCurrency(s.getFirstPrice()).c_str(), -1, &fBodyBold, PointF(statCard.X + 20.0f, statCard.Y + 38.0f), &sfLeft, &bTextWhite);

                g.DrawString(L"10-DAY HIGH", -1, &fSmallBold, PointF(statCard.X + sColW + 20.0f, statCard.Y + 18.0f), &sfLeft, &bTextSecondary);
                g.DrawString(formatCurrency(s.getMaxPrice()).c_str(), -1, &fBodyBold, PointF(statCard.X + sColW + 20.0f, statCard.Y + 38.0f), &sfLeft, &bGreen);

                g.DrawString(L"10-DAY LOW", -1, &fSmallBold, PointF(statCard.X + sColW * 2 + 20.0f, statCard.Y + 18.0f), &sfLeft, &bTextSecondary);
                g.DrawString(formatCurrency(s.getMinPrice()).c_str(), -1, &fBodyBold, PointF(statCard.X + sColW * 2 + 20.0f, statCard.Y + 38.0f), &sfLeft, &bRed);

                g.DrawString(L"10-DAY AVERAGE", -1, &fSmallBold, PointF(statCard.X + sColW * 3 + 20.0f, statCard.Y + 18.0f), &sfLeft, &bTextSecondary);
                g.DrawString(formatCurrency(s.getAveragePrice()).c_str(), -1, &fBodyBold, PointF(statCard.X + sColW * 3 + 20.0f, statCard.Y + 38.0f), &sfLeft, &bAmber);
            }
        }
        else if (currentTab == TAB_PREDICTOR) {
            // === TAB 3: AI PRICE PREDICTOR STUDIO ===

            // CLEAN TWO-ROW CONTROLS BAR (ZERO OVERLAP!)
            // Row 1: Stock Tickers
            float row1Y = contY;
            float rowH = 32.0f;

            g.DrawString(L"EQUITY TICKER:", -1, &fSmallBold, PointF(contX, row1Y + rowH / 2.0f), &sfLeft, &bTextSecondary);

            float pX = contX + 120.0f;
            for (int i = 0; i < (int)stocks.size(); i++) {
                bool sel = (i == selectedPredictStock);
                int btnId = BTN_STOCK_BASE + i;
                float pW = 88.0f;
                RectF pRect(pX, row1Y, pW, rowH);
                g_clickables.push_back({ pRect, btnId, "" });

                bool hov = (hoveredButtonId == btnId);
                SolidBrush bPillBg(sel ? Color(255, 139, 92, 246) : (hov ? colCardHover : colCard));
                drawRoundedRect(g, sel ? &pPurple : &pBorder, &bPillBg, pRect.X, pRect.Y, pRect.Width, pRect.Height, 6.0f);

                g.DrawString(toWide(stocks[i].getSymbol()).c_str(), -1, sel ? &fBodyBold : &fBody,
                             PointF(pRect.X + pW / 2.0f, pRect.Y + rowH / 2.0f), &sfCenter, sel ? &bTextWhite : &bTextWhite);

                pX += pW + 10.0f;
            }

            // Row 2: Prediction Model Buttons (Placed on their own clean line, NEVER overlaps!)
            float row2Y = row1Y + rowH + 10.0f;
            g.DrawString(L"PREDICTION MODEL:", -1, &fSmallBold, PointF(contX, row2Y + rowH / 2.0f), &sfLeft, &bTextSecondary);

            float mX = contX + 150.0f;
            float modW = 180.0f;

            // Model Button 1: Moving Average
            RectF rModelSMA(mX, row2Y, modW, rowH);
            g_clickables.push_back({ rModelSMA, BTN_MODEL_SMA, "" });
            bool smaActive = (currentModel == MODEL_MOVING_AVERAGE);
            SolidBrush bSmaBg(smaActive ? Color(255, 245, 158, 11) : colCard);
            drawRoundedRect(g, smaActive ? &pAmber : &pBorder, &bSmaBg, rModelSMA.X, rModelSMA.Y, rModelSMA.Width, rModelSMA.Height, 6.0f);
            g.DrawString(L"5-Day Moving Average", -1, smaActive ? &fSmallBold : &fSmall,
                         PointF(rModelSMA.X + modW / 2.0f, rModelSMA.Y + rowH / 2.0f), &sfCenter, smaActive ? &bCard : &bTextWhite);

            // Model Button 2: Linear Regression
            RectF rModelLR(mX + modW + 12.0f, row2Y, modW, rowH);
            g_clickables.push_back({ rModelLR, BTN_MODEL_LR, "" });
            bool lrActive = (currentModel == MODEL_LINEAR_REGRESSION);
            SolidBrush bLrBg(lrActive ? Color(255, 139, 92, 246) : colCard);
            drawRoundedRect(g, lrActive ? &pPurple : &pBorder, &bLrBg, rModelLR.X, rModelLR.Y, rModelLR.Width, rModelLR.Height, 6.0f);
            g.DrawString(L"Linear Regression (OLS)", -1, lrActive ? &fSmallBold : &fSmall,
                         PointF(rModelLR.X + modW / 2.0f, rModelLR.Y + rowH / 2.0f), &sfCenter, lrActive ? &bTextWhite : &bTextWhite);

            if (selectedPredictStock >= 0 && selectedPredictStock < (int)stocks.size()) {
                const Stock& s = stocks[selectedPredictStock];
                const auto& prices = s.getPrices();

                double predictedPrice = 0.0;
                double slope = 0.0;
                double intercept = 0.0;
                double rSquared = 0.0;
                double maSum = 0.0;
                int maWindow = 5;

                if (currentModel == MODEL_LINEAR_REGRESSION) {
                    try {
                        auto detail = linearRegressionPredictor.calculateDetail(prices);
                        predictedPrice = detail.nextPrice;
                        slope = detail.slope;
                        intercept = detail.intercept;
                        rSquared = detail.rSquared;
                    } catch (...) {
                        predictedPrice = s.getLatestPrice();
                    }
                } else {
                    try {
                        auto detail = movingAveragePredictor.calculateDetail(prices, 5);
                        predictedPrice = detail.average;
                        maSum = detail.sum;
                        maWindow = detail.window;
                    } catch (...) {
                        predictedPrice = s.getLatestPrice();
                    }
                }

                double latestP = s.getLatestPrice();
                double expectedDelta = predictedPrice - latestP;
                double expectedPct = (latestP > 0) ? (expectedDelta / latestP * 100.0) : 0.0;
                bool isBullish = (expectedDelta >= 0);

                // Two Hero Cards Side by Side (Spacious & Taller)
                float heroRowY = row2Y + rowH + 16.0f;
                float heroCardW = (contW - 16.0f) / 2.0f;
                float heroCardH = 155.0f;

                // Left Hero Card: Predicted Price Output
                RectF cardPred(contX, heroRowY, heroCardW, heroCardH);
                drawRoundedRect(g, isBullish ? &pGreen : &pPurple, &bCard, cardPred.X, cardPred.Y, cardPred.Width, cardPred.Height, 8.0f);

                g.DrawString(L"PROJECTED NEXT PRICE (DAY 11)", -1, &fSmallBold, PointF(cardPred.X + 20.0f, cardPred.Y + 22.0f), &sfLeft, &bTextSecondary);

                // Big Glowing Hero Price
                g.DrawString(formatCurrency(predictedPrice).c_str(), -1, &fHeroPrice, PointF(cardPred.X + 20.0f, cardPred.Y + 58.0f), &sfLeft, isBullish ? &bGreen : &bPurple);

                // Trend badge
                std::wstring trendBadge = (isBullish ? L"▲ EXPECTED SHIFT +" : L"▼ EXPECTED SHIFT ") +
                                          formatCurrency(std::abs(expectedDelta)) + L" (" + formatPercent(expectedPct) + L")";
                g.DrawString(trendBadge.c_str(), -1, &fBodyBold, PointF(cardPred.X + 20.0f, cardPred.Y + 98.0f), &sfLeft, isBullish ? &bGreen : &bAmber);

                std::wstring modelName = (currentModel == MODEL_LINEAR_REGRESSION)
                    ? L"Model: Ordinary Least Squares (OLS) Regression"
                    : L"Model: 5-Period Simple Moving Average (SMA)";
                g.DrawString(modelName.c_str(), -1, &fSmall, PointF(cardPred.X + 20.0f, cardPred.Y + 128.0f), &sfLeft, &bTextMuted);

                // Right Hero Card: Mathematical Breakdown & Formula (Compact, Zero Overflow!)
                RectF cardMath(contX + heroCardW + 16.0f, heroRowY, heroCardW, heroCardH);
                drawRoundedRect(g, &pBorder, &bCard, cardMath.X, cardMath.Y, cardMath.Width, cardMath.Height, 8.0f);

                g.DrawString(L"ALGORITHMIC MODEL EXPLANATION", -1, &fSmallBold, PointF(cardMath.X + 20.0f, cardMath.Y + 22.0f), &sfLeft, &bTextSecondary);

                if (currentModel == MODEL_LINEAR_REGRESSION) {
                    std::wstringstream f1, f2, f3, f4;
                    f1 << L"Trend Line: y = " << std::fixed << std::setprecision(2) << slope << L"·x + " << intercept;
                    f2 << L"Daily Slope (m): " << std::fixed << std::setprecision(2) << slope << L" Rs / session";
                    f3 << L"R² Confidence: " << std::setprecision(1) << (rSquared * 100.0) << L"%  |  Base (c): " << intercept;
                    f4 << L"Next Step: x = 11  ==>  Target: " << formatCurrency(predictedPrice);

                    g.DrawString(f1.str().c_str(), -1, &fFormula, PointF(cardMath.X + 20.0f, cardMath.Y + 48.0f), &sfLeft, &bAccent);
                    g.DrawString(f2.str().c_str(), -1, &fSmallBold, PointF(cardMath.X + 20.0f, cardMath.Y + 74.0f), &sfLeft, &bTextWhite);
                    g.DrawString(f3.str().c_str(), -1, &fSmall, PointF(cardMath.X + 20.0f, cardMath.Y + 100.0f), &sfLeft, &bTextSecondary);
                    g.DrawString(f4.str().c_str(), -1, &fSmallBold, PointF(cardMath.X + 20.0f, cardMath.Y + 128.0f), &sfLeft, isBullish ? &bGreen : &bPurple);
                } else {
                    std::wstringstream f1, f2, f3, f4;
                    f1 << L"SMA Formula: Average of last " << maWindow << L" recorded prices";
                    f2 << L"Window Sum: " << formatCurrency(maSum) << L" (" << maWindow << L" trading days)";
                    f3 << L"Calculation: " << std::fixed << std::setprecision(2) << maSum << L" / " << maWindow;
                    f4 << L"Smoothed Forecast  ==>  Target: " << formatCurrency(predictedPrice);

                    g.DrawString(f1.str().c_str(), -1, &fFormula, PointF(cardMath.X + 20.0f, cardMath.Y + 48.0f), &sfLeft, &bAmber);
                    g.DrawString(f2.str().c_str(), -1, &fSmallBold, PointF(cardMath.X + 20.0f, cardMath.Y + 74.0f), &sfLeft, &bTextWhite);
                    g.DrawString(f3.str().c_str(), -1, &fSmall, PointF(cardMath.X + 20.0f, cardMath.Y + 100.0f), &sfLeft, &bTextSecondary);
                    g.DrawString(f4.str().c_str(), -1, &fSmallBold, PointF(cardMath.X + 20.0f, cardMath.Y + 128.0f), &sfLeft, isBullish ? &bGreen : &bAmber);
                }

                // Combined Forecast Chart (Past + Dotted Projection to Day 11)
                float fcChartY = heroRowY + heroCardH + 16.0f;
                float fcChartH = 250.0f;
                RectF fcCard(contX, fcChartY, contW, fcChartH);
                drawRoundedRect(g, &pBorder, &bCard, fcCard.X, fcCard.Y, fcCard.Width, fcCard.Height, 8.0f);

                float padL = 75.0f;
                float padR = 90.0f; // Wide right padding so Day 11 label never clips
                float padT = 28.0f;
                float padB = 40.0f;

                float cAreaX = fcCard.X + padL;
                float cAreaY = fcCard.Y + padT;
                float cAreaW = fcCard.Width - padL - padR;
                float cAreaH = fcCard.Height - padT - padB;

                int totalPoints = (int)prices.size() + 1; // 10 past + 1 forecast = 11

                double minP = std::min(s.getMinPrice(), predictedPrice);
                double maxP = std::max(s.getMaxPrice(), predictedPrice);
                double range = maxP - minP;
                if (range <= 0.0) range = 1.0;
                minP -= range * 0.12;
                maxP += range * 0.12;
                range = maxP - minP;

                // Grid Lines
                int numG = 4;
                Pen pGrid(Color(255, 26, 38, 56), 1.0f);
                pGrid.SetDashStyle(DashStyleDash);
                for (int k = 0; k <= numG; k++) {
                    float gy = cAreaY + cAreaH * (float)k / (float)numG;
                    double priceVal = maxP - (range * (double)k / (double)numG);
                    g.DrawLine(&pGrid, cAreaX, gy, cAreaX + cAreaW, gy);
                    g.DrawString(formatCurrency(priceVal).c_str(), -1, &fSmall, PointF(fcCard.X + padL - 10.0f, gy), &sfRight, &bTextSecondary);
                }

                // Plot Historical Points (0 to N-1)
                std::vector<PointF> histPts;
                for (size_t i = 0; i < prices.size(); i++) {
                    float px = cAreaX + (cAreaW * (float)i / (float)(totalPoints - 1));
                    float py = cAreaY + cAreaH - (float)(((prices[i] - minP) / range) * cAreaH);
                    histPts.push_back(PointF(px, py));

                    std::wstring dLbl = L"D" + std::to_wstring(i + 1);
                    g.DrawString(dLbl.c_str(), -1, &fSmall, PointF(px, cAreaY + cAreaH + 16.0f), &sfCenter, &bTextSecondary);

                    SolidBrush bPt(colBg);
                    drawRoundedRect(g, &pAccent, &bPt, px - 3.5f, py - 3.5f, 7.0f, 7.0f, 3.5f);
                }

                // Draw Historical Solid Line
                Pen pHist(colAccent, 2.5f);
                for (size_t i = 1; i < histPts.size(); i++) {
                    g.DrawLine(&pHist, histPts[i - 1], histPts[i]);
                }

                // Forecast Point (Day 11)
                float fx = cAreaX + cAreaW;
                float fy = cAreaY + cAreaH - (float)(((predictedPrice - minP) / range) * cAreaH);
                PointF fPt(fx, fy);

                // Dotted Forecast Projection Line from Day 10 to Day 11
                Pen pForecast(isBullish ? colGreen : colPurple, 2.5f);
                pForecast.SetDashStyle(DashStyleDash);
                g.DrawLine(&pForecast, histPts.back(), fPt);

                // Target Glowing Node at Day 11
                SolidBrush bTargetGlow(isBullish ? colGreen : colPurple);
                drawRoundedRect(g, isBullish ? &pGreen : &pPurple, &bTargetGlow, fx - 6.0f, fy - 6.0f, 12.0f, 12.0f, 6.0f);

                // Day 11 Label (Well-spaced, never overlaps D10!)
                g.DrawString(L"★ Day 11", -1, &fSmallBold, PointF(fx, cAreaY + cAreaH + 16.0f), &sfCenter, isBullish ? &bGreen : &bPurple);

                // Floating Target Tag (Auto-clamped inside card)
                float tagW = 105.0f;
                float tagH = 26.0f;
                float tagX = fx - tagW / 2.0f;
                if (tagX + tagW > fcCard.X + fcCard.Width - 10.0f) {
                    tagX = fcCard.X + fcCard.Width - tagW - 10.0f;
                }
                float tagY = fy - tagH - 8.0f;
                if (tagY < cAreaY) tagY = fy + 12.0f;

                RectF tagRect(tagX, tagY, tagW, tagH);
                SolidBrush bTagBg(Color(240, 16, 24, 38));
                drawRoundedRect(g, isBullish ? &pGreen : &pPurple, &bTagBg, tagRect.X, tagRect.Y, tagRect.Width, tagRect.Height, 4.0f);
                g.DrawString(formatCurrency(predictedPrice).c_str(), -1, &fSmallBold,
                             PointF(tagRect.X + tagRect.Width / 2.0f, tagRect.Y + tagRect.Height / 2.0f), &sfCenter, isBullish ? &bGreen : &bPurple);

                // Disclaimer Strip
                float discY = fcChartY + fcChartH + 8.0f;
                g.DrawString(L"⚠️ Note: Algorithmic predictions are mathematical estimates based on historical statistical variance. Market prices are subject to volatility.",
                             -1, &fSmall, PointF(contX + 10.0f, discY + 6.0f), &sfLeft, &bTextMuted);
            }
        }
        else if (currentTab == TAB_COMPARE) {
            // === TAB 4: HEAD-TO-HEAD STOCK COMPARISON ===

            // Selectors Row
            float selRowY = contY;
            float selColW = (contW - 24.0f) / 2.0f;

            // Stock A Picker Bar
            g.DrawString(L"EQUITY 1:", -1, &fSmallBold, PointF(contX, selRowY + 16.0f), &sfLeft, &bAccent);
            float aX = contX + 80.0f;
            for (int i = 0; i < (int)stocks.size(); i++) {
                bool sel = (i == compareStockA);
                int btnId = BTN_STOCK_COMPARE_A_BASE + i;
                float pW = 75.0f;
                RectF pRect(aX, selRowY, pW, 32.0f);
                g_clickables.push_back({ pRect, btnId, "" });
                SolidBrush bPill(sel ? colAccent : colCard);
                drawRoundedRect(g, sel ? &pAccent : &pBorder, &bPill, pRect.X, pRect.Y, pRect.Width, pRect.Height, 5.0f);
                g.DrawString(toWide(stocks[i].getSymbol()).c_str(), -1, &fSmallBold,
                             PointF(pRect.X + pW / 2.0f, pRect.Y + 16.0f), &sfCenter, sel ? &bCard : &bTextWhite);
                aX += pW + 8.0f;
            }

            // Stock B Picker Bar
            float bBaseX = contX + selColW + 24.0f;
            g.DrawString(L"EQUITY 2:", -1, &fSmallBold, PointF(bBaseX, selRowY + 16.0f), &sfLeft, &bPurple);
            float bX = bBaseX + 80.0f;
            for (int i = 0; i < (int)stocks.size(); i++) {
                bool sel = (i == compareStockB);
                int btnId = BTN_STOCK_COMPARE_B_BASE + i;
                float pW = 75.0f;
                RectF pRect(bX, selRowY, pW, 32.0f);
                g_clickables.push_back({ pRect, btnId, "" });
                SolidBrush bPill(sel ? colPurple : colCard);
                drawRoundedRect(g, sel ? &pPurple : &pBorder, &bPill, pRect.X, pRect.Y, pRect.Width, pRect.Height, 5.0f);
                g.DrawString(toWide(stocks[i].getSymbol()).c_str(), -1, &fSmallBold,
                             PointF(pRect.X + pW / 2.0f, pRect.Y + 16.0f), &sfCenter, sel ? &bTextWhite : &bTextWhite);
                bX += pW + 8.0f;
            }

            if (compareStockA >= 0 && compareStockA < (int)stocks.size() &&
                compareStockB >= 0 && compareStockB < (int)stocks.size()) {

                const Stock& stA = stocks[compareStockA];
                const Stock& stB = stocks[compareStockB];

                // Side by Side Cards
                float compCardY = selRowY + 46.0f;
                float compCardH = 140.0f;

                // Card A
                RectF cA(contX, compCardY, selColW, compCardH);
                drawRoundedRect(g, &pAccent, &bCard, cA.X, cA.Y, cA.Width, cA.Height, 8.0f);
                g.DrawString(toWide(stA.getCompany()).c_str(), -1, &fSectionTitle, PointF(cA.X + 20.0f, cA.Y + 26.0f), &sfLeft, &bTextWhite);
                g.DrawString(toWide(stA.getSymbol()).c_str(), -1, &fSmallBold, PointF(cA.X + 20.0f, cA.Y + 50.0f), &sfLeft, &bAccent);
                g.DrawString(formatCurrency(stA.getLatestPrice()).c_str(), -1, &fHeroPrice, PointF(cA.X + 20.0f, cA.Y + 84.0f), &sfLeft, &bTextWhite);

                std::wstring statA = L"10-Day Gain: " + formatPercent(stA.getPriceChangePercent()) + L"  |  Range: " + formatCurrency(stA.getMinPrice()) + L" - " + formatCurrency(stA.getMaxPrice());
                g.DrawString(statA.c_str(), -1, &fSmall, PointF(cA.X + 20.0f, cA.Y + 116.0f), &sfLeft, &bTextSecondary);

                // Card B
                RectF cB(bBaseX, compCardY, selColW, compCardH);
                drawRoundedRect(g, &pPurple, &bCard, cB.X, cB.Y, cB.Width, cB.Height, 8.0f);
                g.DrawString(toWide(stB.getCompany()).c_str(), -1, &fSectionTitle, PointF(cB.X + 20.0f, cB.Y + 26.0f), &sfLeft, &bTextWhite);
                g.DrawString(toWide(stB.getSymbol()).c_str(), -1, &fSmallBold, PointF(cB.X + 20.0f, cB.Y + 50.0f), &sfLeft, &bPurple);
                g.DrawString(formatCurrency(stB.getLatestPrice()).c_str(), -1, &fHeroPrice, PointF(cB.X + 20.0f, cB.Y + 84.0f), &sfLeft, &bTextWhite);

                std::wstring statB = L"10-Day Gain: " + formatPercent(stB.getPriceChangePercent()) + L"  |  Range: " + formatCurrency(stB.getMinPrice()) + L" - " + formatCurrency(stB.getMaxPrice());
                g.DrawString(statB.c_str(), -1, &fSmall, PointF(cB.X + 20.0f, cB.Y + 116.0f), &sfLeft, &bTextSecondary);

                // Middle Comparative Verdict Card
                float verdY = compCardY + compCardH + 16.0f;
                float verdH = 68.0f;
                RectF verdCard(contX, verdY, contW, verdH);
                drawRoundedRect(g, &pBorder, &bCard, verdCard.X, verdCard.Y, verdCard.Width, verdCard.Height, 8.0f);

                double priceDiff = stA.getLatestPrice() - stB.getLatestPrice();
                std::wstring verdict;
                if (priceDiff > 0) {
                    verdict = toWide(stA.getSymbol()) + L" is currently " + formatCurrency(priceDiff) + L" higher priced than " + toWide(stB.getSymbol());
                } else if (priceDiff < 0) {
                    verdict = toWide(stB.getSymbol()) + L" is currently " + formatCurrency(-priceDiff) + L" higher priced than " + toWide(stA.getSymbol());
                } else {
                    verdict = toWide(stA.getSymbol()) + L" and " + toWide(stB.getSymbol()) + L" have identical market valuations.";
                }

                g.DrawString(L"VALUATION COMPARISON VERDICT", -1, &fSmallBold, PointF(verdCard.X + 20.0f, verdCard.Y + 20.0f), &sfLeft, &bTextSecondary);
                g.DrawString(verdict.c_str(), -1, &fSectionTitle, PointF(verdCard.X + 20.0f, verdCard.Y + 44.0f), &sfLeft, &bAccent);

                // Comparative Normalized Performance Chart
                float compChartY = verdY + verdH + 16.0f;
                float compChartH = 260.0f;
                RectF ccCard(contX, compChartY, contW, compChartH);
                drawRoundedRect(g, &pBorder, &bCard, ccCard.X, ccCard.Y, ccCard.Width, ccCard.Height, 8.0f);

                // Chart Legend
                g.DrawString(L"NORMALIZED % GROWTH TRAJECTORY (10 SESSIONS)", -1, &fSmallBold, PointF(ccCard.X + 20.0f, ccCard.Y + 20.0f), &sfLeft, &bTextSecondary);

                // Legend Pills
                float legX = ccCard.X + ccCard.Width - 220.0f;
                drawRoundedRect(g, NULL, &bAccent, legX, ccCard.Y + 15.0f, 10.0f, 10.0f, 2.0f);
                g.DrawString(toWide(stA.getSymbol()).c_str(), -1, &fSmallBold, PointF(legX + 16.0f, ccCard.Y + 20.0f), &sfLeft, &bTextWhite);

                drawRoundedRect(g, NULL, &bPurple, legX + 90.0f, ccCard.Y + 15.0f, 10.0f, 10.0f, 2.0f);
                g.DrawString(toWide(stB.getSymbol()).c_str(), -1, &fSmallBold, PointF(legX + 106.0f, ccCard.Y + 20.0f), &sfLeft, &bTextWhite);

                const auto& prA = stA.getPrices();
                const auto& prB = stB.getPrices();

                if (prA.size() >= 2 && prB.size() >= 2 && prA[0] > 0.0 && prB[0] > 0.0) {
                    float padL = 75.0f;
                    float padR = 45.0f;
                    float padT = 46.0f;
                    float padB = 36.0f;

                    float cAreaX = ccCard.X + padL;
                    float cAreaY = ccCard.Y + padT;
                    float cAreaW = ccCard.Width - padL - padR;
                    float cAreaH = ccCard.Height - padT - padB;

                    std::vector<double> pctA, pctB;
                    for (double val : prA) pctA.push_back(((val - prA[0]) / prA[0]) * 100.0);
                    for (double val : prB) pctB.push_back(((val - prB[0]) / prB[0]) * 100.0);

                    double minPct = 0.0;
                    double maxPct = 0.0;
                    for (double val : pctA) { minPct = std::min(minPct, val); maxPct = std::max(maxPct, val); }
                    for (double val : pctB) { minPct = std::min(minPct, val); maxPct = std::max(maxPct, val); }

                    double pRange = maxPct - minPct;
                    if (pRange <= 0.0) pRange = 2.0;
                    minPct -= pRange * 0.15;
                    maxPct += pRange * 0.15;
                    pRange = maxPct - minPct;

                    // Grid
                    Pen pGrid(Color(255, 26, 38, 56), 1.0f);
                    pGrid.SetDashStyle(DashStyleDash);
                    for (int k = 0; k <= 4; k++) {
                        float gy = cAreaY + cAreaH * (float)k / 4.0f;
                        double pctVal = maxPct - (pRange * (double)k / 4.0f);
                        g.DrawLine(&pGrid, cAreaX, gy, cAreaX + cAreaW, gy);
                        g.DrawString(formatPercent(pctVal).c_str(), -1, &fSmall, PointF(ccCard.X + padL - 10.0f, gy), &sfRight, &bTextSecondary);
                    }

                    // Zero baseline
                    float zeroY = cAreaY + cAreaH - (float)(((0.0 - minPct) / pRange) * cAreaH);
                    Pen pZero(Color(255, 70, 85, 110), 1.5f);
                    g.DrawLine(&pZero, cAreaX, zeroY, cAreaX + cAreaW, zeroY);

                    // Plot Stock A (Cyan)
                    std::vector<PointF> ptsA, ptsB;
                    for (size_t i = 0; i < pctA.size(); i++) {
                        float px = cAreaX + (cAreaW * (float)i / (float)(pctA.size() - 1));
                        float py = cAreaY + cAreaH - (float)(((pctA[i] - minPct) / pRange) * cAreaH);
                        ptsA.push_back(PointF(px, py));
                    }
                    for (size_t i = 0; i < pctB.size(); i++) {
                        float px = cAreaX + (cAreaW * (float)i / (float)(pctB.size() - 1));
                        float py = cAreaY + cAreaH - (float)(((pctB[i] - minPct) / pRange) * cAreaH);
                        ptsB.push_back(PointF(px, py));
                    }

                    Pen pTraceA(colAccent, 2.5f);
                    Pen pTraceB(colPurple, 2.5f);
                    for (size_t i = 1; i < ptsA.size(); i++) g.DrawLine(&pTraceA, ptsA[i - 1], ptsA[i]);
                    for (size_t i = 1; i < ptsB.size(); i++) g.DrawLine(&pTraceB, ptsB[i - 1], ptsB[i]);

                    for (const auto& pt : ptsA) {
                        drawRoundedRect(g, &pAccent, &bBg, pt.X - 3.5f, pt.Y - 3.5f, 7.0f, 7.0f, 3.5f);
                    }
                    for (const auto& pt : ptsB) {
                        drawRoundedRect(g, &pPurple, &bBg, pt.X - 3.5f, pt.Y - 3.5f, 7.0f, 7.0f, 3.5f);
                    }
                }
            }
        }
        else if (currentTab == TAB_PORTFOLIO) {
            // === TAB 5: PORTFOLIO MANAGER & LIVE VALUATION ===

            const auto& holdings = portfolio.getHoldings();
            double totalCost = portfolio.getTotalCost();
            double totalValue = portfolio.getTotalMarketValue(stockManager);
            double totalPL = portfolio.getTotalProfitLoss(stockManager);
            double totalPLPct = portfolio.getTotalProfitLossPercent(stockManager);
            bool isProfitable = (totalPL >= 0);

            // 3 Top Summary Cards (Spacious)
            float pSpacing = 12.0f;
            float pCardW = (contW - (pSpacing * 2.0f)) / 3.0f;
            float pCardH = 95.0f;

            // Card 1: Total Portfolio Value
            RectF pc1(contX, contY, pCardW, pCardH);
            drawRoundedRect(g, &pBorder, &bCard, pc1.X, pc1.Y, pc1.Width, pc1.Height, 8.0f);
            g.DrawString(L"TOTAL PORTFOLIO VALUE", -1, &fSmallBold, PointF(pc1.X + 20.0f, pc1.Y + 20.0f), &sfLeft, &bTextSecondary);
            g.DrawString(formatCurrency(totalValue).c_str(), -1, &fLargeNum, PointF(pc1.X + 20.0f, pc1.Y + 50.0f), &sfLeft, &bAccent);
            g.DrawString(L"Real-time mark-to-market valuation", -1, &fSmall, PointF(pc1.X + 20.0f, pc1.Y + 74.0f), &sfLeft, &bTextMuted);

            // Card 2: Total Invested Capital
            RectF pc2(contX + pCardW + pSpacing, contY, pCardW, pCardH);
            drawRoundedRect(g, &pBorder, &bCard, pc2.X, pc2.Y, pc2.Width, pc2.Height, 8.0f);
            g.DrawString(L"TOTAL INVESTED CAPITAL", -1, &fSmallBold, PointF(pc2.X + 20.0f, pc2.Y + 20.0f), &sfLeft, &bTextSecondary);
            g.DrawString(formatCurrency(totalCost).c_str(), -1, &fLargeNum, PointF(pc2.X + 20.0f, pc2.Y + 50.0f), &sfLeft, &bTextWhite);
            g.DrawString(L"Cumulative acquisition cost", -1, &fSmall, PointF(pc2.X + 20.0f, pc2.Y + 74.0f), &sfLeft, &bTextMuted);

            // Card 3: Unrealized Profit / Loss
            RectF pc3(contX + (pCardW + pSpacing) * 2.0f, contY, pCardW, pCardH);
            drawRoundedRect(g, isProfitable ? &pGreen : &pRed, &bCard, pc3.X, pc3.Y, pc3.Width, pc3.Height, 8.0f);
            g.DrawString(L"UNREALIZED PROFIT / LOSS", -1, &fSmallBold, PointF(pc3.X + 20.0f, pc3.Y + 20.0f), &sfLeft, &bTextSecondary);
            std::wstring plText = (isProfitable ? L"+ " : L"- ") + formatCurrency(std::abs(totalPL)) + L" (" + formatPercent(totalPLPct) + L")";
            g.DrawString(plText.c_str(), -1, &fLargeNum, PointF(pc3.X + 20.0f, pc3.Y + 50.0f), &sfLeft, isProfitable ? &bGreen : &bRed);
            g.DrawString(isProfitable ? L"Bullish Portfolio Net Gain" : L"Bearish Portfolio Drawdown", -1, &fSmall, PointF(pc3.X + 20.0f, pc3.Y + 74.0f), &sfLeft, &bTextMuted);

            // Left: Holdings Table (60% width)
            float tableY = contY + pCardH + 16.0f;
            float tableW = contW * 0.60f;
            float formW = contW - tableW - 16.0f;
            float bottomH = contH - (tableY - contY);

            RectF holdTableCard(contX, tableY, tableW, bottomH);
            drawRoundedRect(g, &pBorder, &bCard, holdTableCard.X, holdTableCard.Y, holdTableCard.Width, holdTableCard.Height, 8.0f);

            g.DrawString(L"CURRENT EQUITY HOLDINGS", -1, &fSectionTitle, PointF(holdTableCard.X + 20.0f, holdTableCard.Y + 24.0f), &sfLeft, &bTextWhite);

            // PROPORTIONAL COLUMNS FOR HOLDINGS TABLE (Zero collision!)
            float hPad = 18.0f;
            float hInnerW = tableW - (hPad * 2.0f);

            float colSym  = holdTableCard.X + hPad;
            float colQty  = colSym  + hInnerW * 0.16f;
            float colBuyP = colQty  + hInnerW * 0.13f;
            float colCurP = colBuyP + hInnerW * 0.18f;
            float colVal  = colCurP + hInnerW * 0.18f;
            float colPL   = colVal  + hInnerW * 0.18f;
            float colAct  = holdTableCard.X + tableW - hPad - 55.0f; // Right-anchored!

            float thRowY = holdTableCard.Y + 48.0f;
            g.DrawLine(&pBorder, holdTableCard.X + 15.0f, thRowY, holdTableCard.X + tableW - 15.0f, thRowY);

            thRowY += 10.0f;
            g.DrawString(L"SYMBOL", -1, &fSmallBold, PointF(colSym, thRowY), &sfLeft, &bTextSecondary);
            g.DrawString(L"SHARES", -1, &fSmallBold, PointF(colQty, thRowY), &sfLeft, &bTextSecondary);
            g.DrawString(L"BUY PRICE", -1, &fSmallBold, PointF(colBuyP, thRowY), &sfLeft, &bTextSecondary);
            g.DrawString(L"MARKET", -1, &fSmallBold, PointF(colCurP, thRowY), &sfLeft, &bTextSecondary);
            g.DrawString(L"VALUE", -1, &fSmallBold, PointF(colVal, thRowY), &sfLeft, &bTextSecondary);
            g.DrawString(L"P&L", -1, &fSmallBold, PointF(colPL, thRowY), &sfLeft, &bTextSecondary);
            g.DrawString(L"ACTION", -1, &fSmallBold, PointF(colAct, thRowY), &sfLeft, &bTextSecondary);

            float hRowY = thRowY + 20.0f;
            g.DrawLine(&pBorder, holdTableCard.X + 15.0f, hRowY, holdTableCard.X + tableW - 15.0f, hRowY);

            hRowY += 8.0f;
            if (holdings.empty()) {
                g.DrawString(L"Your portfolio is empty.", -1, &fSectionTitle, PointF(holdTableCard.X + tableW / 2.0f, holdTableCard.Y + bottomH / 2.0f - 10.0f), &sfCenter, &bTextSecondary);
                g.DrawString(L"Add shares of any equity using the Buy Order card on the right.", -1, &fBody, PointF(holdTableCard.X + tableW / 2.0f, holdTableCard.Y + bottomH / 2.0f + 16.0f), &sfCenter, &bTextMuted);
            } else {
                for (int i = 0; i < (int)holdings.size(); i++) {
                    const auto& h = holdings[i];
                    const Stock* s = stockManager.search(h.symbol);
                    double currentP = s ? s->getLatestPrice() : h.buyPrice;
                    double currentV = h.quantity * currentP;
                    double costV = h.quantity * h.buyPrice;
                    double plVal = currentV - costV;
                    bool pos = (plVal >= 0);

                    float midY = hRowY + 18.0f;

                    // Symbol Badge
                    drawRoundedRect(g, &pAccent, NULL, colSym, midY - 11.0f, 68.0f, 22.0f, 4.0f);
                    g.DrawString(toWide(h.symbol).c_str(), -1, &fSmallBold, PointF(colSym + 34.0f, midY), &sfCenter, &bAccent);

                    // Quantity
                    g.DrawString(std::to_wstring(h.quantity).c_str(), -1, &fBodyBold, PointF(colQty, midY), &sfLeft, &bTextWhite);

                    // Buy Price
                    g.DrawString(formatCurrency(h.buyPrice).c_str(), -1, &fBody, PointF(colBuyP, midY), &sfLeft, &bTextSecondary);

                    // Current Price
                    g.DrawString(formatCurrency(currentP).c_str(), -1, &fBodyBold, PointF(colCurP, midY), &sfLeft, &bTextWhite);

                    // Value
                    g.DrawString(formatCurrency(currentV).c_str(), -1, &fBodyBold, PointF(colVal, midY), &sfLeft, &bTextWhite);

                    // P&L
                    std::wstring plStr = (pos ? L"+" : L"") + formatCurrency(plVal);
                    g.DrawString(plStr.c_str(), -1, &fSmallBold, PointF(colPL, midY), &sfLeft, pos ? &bGreen : &bRed);

                    // Remove Button
                    RectF bRem(colAct, midY - 11.0f, 54.0f, 22.0f);
                    int bRemId = BTN_REMOVE_HOLDING_BASE + i;
                    g_clickables.push_back({ bRem, bRemId, "" });
                    bool remHov = (hoveredButtonId == bRemId);
                    SolidBrush bRemBg(remHov ? Color(255, 60, 20, 30) : Color(255, 36, 18, 25));
                    drawRoundedRect(g, &pRed, &bRemBg, bRem.X, bRem.Y, bRem.Width, bRem.Height, 4.0f);
                    g.DrawString(L"Sell", -1, &fSmallBold, PointF(bRem.X + 27.0f, midY), &sfCenter, &bRed);

                    hRowY += 40.0f;
                }
            }

            // Right: Add / Buy Order Card (Spacious layout)
            float formX = contX + tableW + 16.0f;
            RectF formCard(formX, tableY, formW, bottomH);
            drawRoundedRect(g, &pBorder, &bCard, formCard.X, formCard.Y, formCard.Width, formCard.Height, 8.0f);

            g.DrawString(L"EXECUTE BUY ORDER", -1, &fSectionTitle, PointF(formX + 20.0f, tableY + 24.0f), &sfLeft, &bTextWhite);
            g.DrawString(L"Add or increase equity positions", -1, &fSmall, PointF(formX + 20.0f, tableY + 46.0f), &sfLeft, &bTextSecondary);

            float fY = tableY + 72.0f;

            // 1. Symbol Selection Chips
            g.DrawString(L"CHOOSE EQUITY TICKER:", -1, &fSmallBold, PointF(formX + 20.0f, fY), &sfLeft, &bTextSecondary);
            fY += 18.0f;

            float chipX = formX + 20.0f;
            for (int i = 0; i < (int)stocks.size(); i++) {
                bool sel = (i == portfolioStockIndex);
                int btnId = BTN_STOCK_PORT_BASE + i;
                float chW = (formW - 56.0f) / 3.0f;
                if (i == 3) {
                    chipX = formX + 20.0f;
                    fY += 34.0f;
                }
                RectF chRect(chipX, fY, chW, 28.0f);
                g_clickables.push_back({ chRect, btnId, "" });
                SolidBrush bCh(sel ? colAccent : Color(255, 18, 26, 40));
                drawRoundedRect(g, sel ? &pAccent : &pBorder, &bCh, chRect.X, chRect.Y, chRect.Width, chRect.Height, 4.0f);
                g.DrawString(toWide(stocks[i].getSymbol()).c_str(), -1, &fSmallBold,
                             PointF(chRect.X + chW / 2.0f, chRect.Y + 14.0f), &sfCenter, sel ? &bCard : &bTextWhite);
                chipX += chW + 8.0f;
            }

            fY += 46.0f;

            // 2. Shares Quantity Controls
            g.DrawString(L"QUANTITY (SHARES):", -1, &fSmallBold, PointF(formX + 20.0f, fY), &sfLeft, &bTextSecondary);
            fY += 18.0f;

            // Display Box
            RectF qtyBox(formX + 20.0f, fY, 86.0f, 32.0f);
            drawRoundedRect(g, &pBorder, &bSidebar, qtyBox.X, qtyBox.Y, qtyBox.Width, qtyBox.Height, 4.0f);
            g.DrawString(std::to_wstring(portfolioQuantity).c_str(), -1, &fLargeNum,
                         PointF(qtyBox.X + qtyBox.Width / 2.0f, qtyBox.Y + 16.0f), &sfCenter, &bAccent);

            // Increment / Decrement Buttons
            struct QBtn { int id; const wchar_t* lbl; float w; };
            QBtn qBtns[] = {
                { BTN_QTY_M10, L"-10", 38.0f },
                { BTN_QTY_M1,  L"-1",  32.0f },
                { BTN_QTY_P1,  L"+1",  32.0f },
                { BTN_QTY_P10, L"+10", 38.0f }
            };
            float qbX = formX + 116.0f;
            for (const auto& qb : qBtns) {
                RectF rB(qbX, fY, qb.w, 32.0f);
                g_clickables.push_back({ rB, qb.id, "" });
                bool hov = (hoveredButtonId == qb.id);
                SolidBrush bB(hov ? colCardHover : Color(255, 24, 35, 54));
                drawRoundedRect(g, &pBorderLight, &bB, rB.X, rB.Y, rB.Width, rB.Height, 4.0f);
                g.DrawString(qb.lbl, -1, &fSmallBold, PointF(rB.X + qb.w / 2.0f, rB.Y + 16.0f), &sfCenter, &bTextWhite);
                qbX += qb.w + 6.0f;
            }

            fY += 48.0f;

            // 3. Buy Price Controls
            g.DrawString(L"PRICE PER SHARE:", -1, &fSmallBold, PointF(formX + 20.0f, fY), &sfLeft, &bTextSecondary);
            fY += 18.0f;

            // Price Display Box
            RectF prBox(formX + 20.0f, fY, 115.0f, 32.0f);
            drawRoundedRect(g, &pBorder, &bSidebar, prBox.X, prBox.Y, prBox.Width, prBox.Height, 4.0f);
            g.DrawString(formatCurrency(portfolioBuyPrice).c_str(), -1, &fBodyBold,
                         PointF(prBox.X + prBox.Width / 2.0f, prBox.Y + 16.0f), &sfCenter, &bGreen);

            // Price Increment Buttons
            QBtn pBtns[] = {
                { BTN_PRICE_M10, L"-10", 38.0f },
                { BTN_PRICE_M1,  L"-1",  32.0f },
                { BTN_PRICE_P1,  L"+1",  32.0f },
                { BTN_PRICE_P10, L"+10", 38.0f }
            };
            float pbX = formX + 144.0f;
            for (const auto& pb : pBtns) {
                RectF rB(pbX, fY, pb.w, 32.0f);
                g_clickables.push_back({ rB, pb.id, "" });
                bool hov = (hoveredButtonId == pb.id);
                SolidBrush bB(hov ? colCardHover : Color(255, 24, 35, 54));
                drawRoundedRect(g, &pBorderLight, &bB, rB.X, rB.Y, rB.Width, rB.Height, 4.0f);
                g.DrawString(pb.lbl, -1, &fSmallBold, PointF(rB.X + pb.w / 2.0f, rB.Y + 16.0f), &sfCenter, &bTextWhite);
                pbX += pb.w + 6.0f;
            }

            fY += 50.0f;

            // Two-Tier Total Cost Preview Box (Spacious, Zero Collision!)
            double totalOrderCost = portfolioQuantity * portfolioBuyPrice;
            RectF ordPreview(formX + 20.0f, fY, formW - 40.0f, 56.0f);
            drawRoundedRect(g, &pBorder, &bSidebar, ordPreview.X, ordPreview.Y, ordPreview.Width, ordPreview.Height, 6.0f);
            g.DrawString(L"ESTIMATED TOTAL ORDER VALUE", -1, &fSmallBold, PointF(ordPreview.X + 16.0f, ordPreview.Y + 15.0f), &sfLeft, &bTextSecondary);
            g.DrawString(formatCurrency(totalOrderCost).c_str(), -1, &fLargeNum, PointF(ordPreview.X + 16.0f, ordPreview.Y + 38.0f), &sfLeft, &bAccent);

            fY += 68.0f;

            // Submit Button: Add to Portfolio
            RectF btnAdd(formX + 20.0f, fY, formW - 40.0f, 42.0f);
            g_clickables.push_back({ btnAdd, BTN_PORTFOLIO_ADD, "" });
            bool addHov = (hoveredButtonId == BTN_PORTFOLIO_ADD);
            SolidBrush bAdd(addHov ? Color(255, 14, 165, 233) : colAccent);
            drawRoundedRect(g, &pAccent, &bAdd, btnAdd.X, btnAdd.Y, btnAdd.Width, btnAdd.Height, 6.0f);
            g.DrawString(L"+ EXECUTE & ADD TO PORTFOLIO", -1, &fBodyBold,
                         PointF(btnAdd.X + btnAdd.Width / 2.0f, btnAdd.Y + 21.0f), &sfCenter, &bCard);
        }

        // 5. Toast Notification Banner
        if (!toastMessage.empty()) {
            float toastW = 440.0f;
            float toastH = 40.0f;
            float toastX = (float)width - toastW - 24.0f;
            float toastY = (float)height - toastH - 20.0f;

            RectF tRect(toastX, toastY, toastW, toastH);
            SolidBrush bToastBg(Color(250, 16, 24, 38));
            Pen* pToast = toastError ? &pRed : &pGreen;
            drawRoundedRect(g, pToast, &bToastBg, tRect.X, tRect.Y, tRect.Width, tRect.Height, 6.0f);

            std::wstring tText = (toastError ? L"Warning: " : L"Success: ") + toWide(toastMessage);
            g.DrawString(tText.c_str(), -1, &fSmallBold, PointF(tRect.X + 16.0f, tRect.Y + 20.0f), &sfLeft, toastError ? &bRed : &bGreen);
        }
    } catch (...) {}

    // Copy to Window Screen
    BitBlt(hdc, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY);
    EndPaint(hwnd, &ps);
}

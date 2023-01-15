
#include <string>
#include <vector>
#include <Windows.h>
#include <iostream>
#include "capturer.h"

static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam);

struct CallbackParams
{
    std::vector<AppRecord>* pApps;
    HWND activeWindow;
};

void getOpenedApps(std::vector<AppRecord> *apps)
{
    struct CallbackParams lparam;
    lparam.pApps = apps;
    lparam.activeWindow = GetForegroundWindow();
    EnumWindows(enumWindowCallback, reinterpret_cast<LPARAM>(&lparam));
}

static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam)
{
    struct CallbackParams* callbackParams = reinterpret_cast<CallbackParams*>(lparam);
    auto *apps = callbackParams->pApps;
    HWND activeWindow = callbackParams->activeWindow;

    int titleLength = GetWindowTextLength(hWnd);
    char *bufferTitle = new char[titleLength + 1];
    GetWindowTextA(hWnd, bufferTitle, titleLength + 1);
    std::string title(bufferTitle);
    delete[] bufferTitle;

    LONG exStyles = GetWindowLongA(hWnd, GWL_EXSTYLE);
    LONG styles = GetWindowLongA(hWnd, GWL_STYLE);
    bool isCaption = (styles & WS_CAPTION) != 0;
    bool isOverlapped = (styles & WS_OVERLAPPEDWINDOW) != 0;

    if (IsWindowVisible(hWnd) && isCaption && isOverlapped && titleLength != 0)
    {
        AppRecord appRecord;
        appRecord.path = "<UNKNOWN>";
        appRecord.title = title;
        appRecord.isActive = hWnd == activeWindow;
        apps->push_back(appRecord);
    }
    return TRUE;
}
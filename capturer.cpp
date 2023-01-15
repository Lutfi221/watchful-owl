
#include <string>
#include <vector>
#include <Windows.h>
#include <iostream>
#include "capturer.h"
#include "helpers.h"

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
    wchar_t* bufferTitle = new wchar_t[titleLength + 1];
    GetWindowTextW(hWnd, bufferTitle, titleLength + 1);
    std::wstring title(bufferTitle);
    delete[] bufferTitle;

    LONG exStyles = GetWindowLongW(hWnd, GWL_EXSTYLE);
    LONG styles = GetWindowLongW(hWnd, GWL_STYLE);
    bool isCaption = (styles & WS_CAPTION) != 0;
    bool isOverlapped = (styles & WS_OVERLAPPEDWINDOW) != 0;

    if (IsWindowVisible(hWnd) && isCaption && isOverlapped && titleLength != 0)
    {
        AppRecord appRecord;
        appRecord.path = "<UNKNOWN>";
        appRecord.title = toUtf8(title);
        appRecord.isActive = hWnd == activeWindow;
        apps->push_back(appRecord);
    }
    return TRUE;
}
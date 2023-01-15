
#include <string>
#include <vector>
#include <Windows.h>
#include "capturer.h"

static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam);

void getOpenedApps(std::vector<AppRecord>* apps) {
    EnumWindows(enumWindowCallback, reinterpret_cast<LPARAM>(apps));
}

static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam)
{
    std::vector<AppRecord>* apps = reinterpret_cast<std::vector<AppRecord>*>(lparam);

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
        apps->push_back(appRecord);
    }
    return TRUE;
}
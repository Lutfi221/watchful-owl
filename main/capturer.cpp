
#include <string>
#include <vector>
#include <Windows.h>
#include <psapi.h>
#include "capturer.h"
#include "helpers.h"

static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam);
std::wstring getWindowProcessPath(HWND hWnd);

struct CallbackParams
{
    std::vector<AppRecord> *pApps;
    HWND activeWindow;
};

void getOpenedApps(std::vector<AppRecord> *apps)
{
    struct CallbackParams lparam;
    lparam.pApps = apps;
    lparam.activeWindow = GetForegroundWindow();
    EnumWindows(enumWindowCallback, reinterpret_cast<LPARAM>(&lparam));
}

/// @brief Gets the duration (in seconds) since last user input.
UINT getDurationSinceLastInput()
{
    LASTINPUTINFO lastInput;
    lastInput.cbSize = sizeof(lastInput);
    GetLastInputInfo(&lastInput);
    return (GetTickCount() - lastInput.dwTime) / 1000;
}

static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam)
{
    struct CallbackParams *callbackParams = reinterpret_cast<CallbackParams *>(lparam);
    auto *apps = callbackParams->pApps;
    HWND activeWindow = callbackParams->activeWindow;

    int titleLength = GetWindowTextLength(hWnd);
    wchar_t *bufferTitle = new wchar_t[titleLength + 1];
    GetWindowTextW(hWnd, bufferTitle, titleLength + 1);
    std::wstring title(bufferTitle);
    delete[] bufferTitle;

    LONG exStyles = GetWindowLongW(hWnd, GWL_EXSTYLE);
    LONG styles = GetWindowLongW(hWnd, GWL_STYLE);
    bool isCaption = (styles & WS_CAPTION) != 0;
    bool isOverlapped = (styles & WS_OVERLAPPEDWINDOW) != 0;
    bool isActive = hWnd == activeWindow;

    if ((IsWindowVisible(hWnd) && isCaption &&
         isOverlapped && titleLength != 0) ||
        isActive)
    {
        AppRecord appRecord;
        appRecord.path = toUtf8(getWindowProcessPath(hWnd));
        appRecord.title = toUtf8(title);
        appRecord.isActive = isActive;
        apps->push_back(appRecord);
    }
    return TRUE;
}

std::wstring getWindowProcessPath(HWND hWnd)
{
    DWORD pId;
    HANDLE processHandle = NULL;
    WCHAR processPath[MAX_PATH];

    GetWindowThreadProcessId(hWnd, &pId);
    processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pId);
    if (processHandle != NULL)
    {
        auto n = GetModuleFileNameExW(processHandle, NULL, processPath, MAX_PATH);
        CloseHandle(processHandle);
        if (n == 0)
            return L"";
    }
    else
        return L"";

    return processPath;
}
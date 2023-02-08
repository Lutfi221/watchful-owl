#include <string>
#include <vector>
#include <codecvt>
#include <locale>
#include <Windows.h>
#include <tlhelp32.h>
#include "helpers.h"
#include <filesystem>

// https://gist.github.com/gchudnov/c1ba72d45e394180e22f
std::string toUtf8(const std::wstring &wide)
{
    std::u16string u16str(wide.begin(), wide.end());
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    std::string utf8 = convert.to_bytes(u16str);
    return utf8;
}

// Determines if path is relative.
bool isPathRelative(const std::string path)
{
    const auto x = path[0];
    return (x == '.' || x != '/' || x != '\\');
}

std::string getExecutablePath()
{
    CHAR path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string s(path);
    return s;
}

std::string getExecutableDirPath()
{
    std::filesystem::path p(getExecutablePath());
    return p.remove_filename().u8string();
}

/// @brief Get process ids with the specified process name.
/// @param processName
/// @return List of process ids
std::vector<DWORD> getProcessIds(const std::string &processName)
{
    std::vector<DWORD> ids;
    PROCESSENTRY32 processInfo;
    processInfo.dwSize = sizeof(processInfo);

    HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (processesSnapshot == INVALID_HANDLE_VALUE)
        return ids;

    Process32First(processesSnapshot, &processInfo);
    if (!processName.compare(processInfo.szExeFile))
    {
        CloseHandle(processesSnapshot);
        ids.push_back(processInfo.th32ProcessID);
    }

    while (Process32Next(processesSnapshot, &processInfo))
    {
        if (!processName.compare(processInfo.szExeFile))
        {
            CloseHandle(processesSnapshot);
            ids.push_back(processInfo.th32ProcessID);
        }
    }

    CloseHandle(processesSnapshot);
    return ids;
}

// @brief Starts a program.
// The program will run independently, and will keep running
// even after the caller has terminated.
//
// https://stackoverflow.com/a/38158534
void startProgram(std::string path)
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    CreateProcessA(
        path.c_str(),
        NULL,               // Command line
        NULL,               // Process handle not inheritable
        NULL,               // Thread handle not inheritable
        FALSE,              // Set handle inheritance to FALSE
        CREATE_NEW_CONSOLE, // Opens file in a separate console
        NULL,               // Use parent's environment block
        NULL,               // Use parent's starting directory
        &si,                // Pointer to STARTUPINFO structure
        &pi                 // Pointer to PROCESS_INFORMATION structure
    );
}

void killProcess(DWORD processId)
{
    const auto explorer = OpenProcess(PROCESS_TERMINATE, false, processId);
    TerminateProcess(explorer, 1);
    CloseHandle(explorer);
}

/// @brief Kill other running perpetual instances except the current process.
void killOtherPerpetualInstances()
{
    DWORD currentPId = GetCurrentProcessId();
    auto ids = getProcessIds("perpetual-owl.exe");
    for (int i = 0; i < ids.size(); i++)
    {
        if (currentPId == ids[i])
            continue;
        killProcess(ids[i]);
    }
}

/// @brief Kill all running perpetual instances.
void killAllPerpetualInstances()
{
    auto ids = getProcessIds("perpetual-owl.exe");
    for (int i = 0; i < ids.size(); i++)
        killProcess(ids[i]);
}

bool isPerpetualInstanceRunning()
{
    auto ids = getProcessIds("perpetual-owl.exe");
    return ids.size() > 0;
}
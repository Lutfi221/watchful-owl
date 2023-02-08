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
    return path;
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

void killProcess(DWORD processId)
{
    const auto explorer = OpenProcess(PROCESS_TERMINATE, false, processId);
    TerminateProcess(explorer, 1);
    CloseHandle(explorer);
}

/// @brief Kill other running perpetual instances.
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
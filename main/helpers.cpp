#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <regex>
#include <string>
#include <tlhelp32.h>
#include <vector>

#include "constants.hpp"
#include "dev-logger.h"
#include "helpers.h"

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

std::filesystem::path getExecutablePath()
{
    CHAR path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::filesystem::path(path);
}

std::filesystem::path getExecutableDirPath()
{
    return getExecutablePath().remove_filename();
}

std::filesystem::path prepareAndProcessPath(std::string path, bool createDirs, bool isDir)
{
    return prepareAndProcessPath(std::filesystem::path(path), createDirs, isDir);
}
std::filesystem::path prepareAndProcessPath(std::filesystem::path in, bool createDirs, bool isDir)
{
    using namespace std::filesystem;
    path out;
    if (in.is_relative())
        out = getExecutableDirPath() / in;
    else
        out = in;

    if (createDirs)
        if (isDir)
            create_directories(out);
        else
            create_directories(out.parent_path());

    return std::filesystem::weakly_canonical(out);
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

// https://stackoverflow.com/a/116220
std::string readFile(std::string_view path)
{
    constexpr auto read_size = std::size_t(4096);
    auto stream = std::ifstream(path.data());
    stream.exceptions(std::ios_base::badbit);

    auto out = std::string();
    auto buf = std::string(read_size, '\0');
    while (stream.read(&buf[0], read_size))
    {
        out.append(buf, 0, stream.gcount());
    }
    out.append(buf, 0, stream.gcount());
    return out;
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

    INFO("Starting program `{}`", path);
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
    auto ids = getProcessIds(constants::PERPETUAL_EXE_FILENAME);
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
    INFO("Killing all perpetual instances");
    auto ids = getProcessIds(constants::PERPETUAL_EXE_FILENAME);
    for (int i = 0; i < ids.size(); i++)
    {
        DEBUG("Killing process with id {}", ids[i]);
        killProcess(ids[i]);
    }
}

bool isPerpetualInstanceRunning()
{
    auto ids = getProcessIds(constants::PERPETUAL_EXE_FILENAME);
    return ids.size() > 0;
}

std::vector<std::filesystem::path> getFileListByRegex(std::filesystem::path dir, std::regex pattern)
{
    using namespace std;
    if (!filesystem::is_directory(dir))
        throw runtime_error(dir.string() + "is not a folder.");

    vector<filesystem::path> matchedFiles;

    INFO("Get files in `{}` that match regex pattern", dir.string());
    for (const auto &entry : filesystem::directory_iterator(dir))
    {
        if (entry.is_regular_file())
        {
            const auto baseName = entry.path().filename().string();
            if (regex_match(baseName, pattern))
                matchedFiles.push_back(entry.path());
        }
    }

    return matchedFiles;
};
#include <Windows.h>
#include <cstring>
#include <filesystem>
#include <fstream>

#include "autorun.h"
#include "constants.hpp"
#include "helpers.h"

/// @brief Get the path to the autorunner.
/// (Might not exist)
std::filesystem::path getRunnerPath()
{
    namespace fs = std::filesystem;
    CHAR lpBuffer[MAX_PATH];
    GetEnvironmentVariableA("APPDATA", lpBuffer, MAX_PATH);
    auto appdataPath = fs::path(lpBuffer);
    auto runnerPartialPath = fs::path(".\\Microsoft\\Windows\\Start Menu\\"
                                      "Programs\\Startup\\Watchful Owl Perpetual"
#ifdef DEBUG_BUILD
                                      "-DEBUG"
#endif
                                      ".bat");
    return appdataPath / runnerPartialPath;
}

const std::filesystem::path RUNNER_PATH = getRunnerPath();

std::string generateAutorunBatContent()
{
    std::string p = (getExecutableDirPath() /
                     std::filesystem::path(constants::PERPETUAL_EXE_FILENAME))
                        .u8string();
    return u8"start \"\" \"" + p + u8"\"";
}

AutorunStatus getAutorunStatus()
{
    auto runnerFileExists = std::filesystem::exists(RUNNER_PATH);
    if (!runnerFileExists)
        return AutorunDisabled;
    auto runnerFileContent = readFile(RUNNER_PATH.u8string());
    if (runnerFileContent == generateAutorunBatContent())
        return AutorunEnabled;
    return AutorunInvalid;
}

void enableAutorun()
{
    std::ofstream batFile;
    batFile.open(RUNNER_PATH, std::ios::out | std::ofstream::trunc);
    if (batFile.fail())
        throw std::ios_base::failure(std::strerror(errno));
    batFile << generateAutorunBatContent();
}

void disableAutorun()
{
    std::filesystem::remove(RUNNER_PATH);
}
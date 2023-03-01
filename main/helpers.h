#ifndef MAIN_HELPERS
#define MAIN_HELPERS
#include <chrono>
#include <filesystem>
#include <regex>
#include <string>
#include <thread>

std::string toUtf8(const std::wstring &wide);

bool isPathRelative(const std::string path);

std::filesystem::path getExecutablePath();
std::filesystem::path getExecutableDirPath();

inline bool fileExists(const std::string &filepath)
{
    using namespace std::filesystem;
    if (isPathRelative(filepath))
    {
        path p = getExecutableDirPath() / path(filepath);
        return exists(p);
    }
    return exists(path(filepath));
}

/// @brief Create a processed path that is weakly canonical, and absolute.
///        A relative path is considered relative to the main program executable path.
///        This does not mutate path.
/// @param path Path to be processed.
/// @param createDirs Should directories be created if they're missing.
/// @param isDir Does the path point to a directory?
/// @return Processed path
std::filesystem::path prepareAndProcessPath(
    std::string path, bool createDirs = true,
    bool isDir = false);
/// @brief Create a processed path that is weakly canonical, and absolute.
///        A relative path is considered relative to the main program executable path.
///        This does not mutate path.
/// @param path Path to be processed.
/// @param createDirs Should directories be created if they're missing.
/// @param isDir Does the path point to a directory?
/// @return Processed path
std::filesystem::path prepareAndProcessPath(
    std::filesystem::path path, bool createDirs = true,
    bool isDir = false);

void killOtherPerpetualInstances();
void killAllPerpetualInstances();
bool isPerpetualInstanceRunning();

inline void sleepFor(unsigned int seconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

std::string readFile(std::string_view path);
void startProgram(std::string path);

std::vector<std::filesystem::path> getFileListByRegex(
    std::filesystem::path dir, std::regex pattern);

#endif /* MAIN_HELPERS */

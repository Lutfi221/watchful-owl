#ifndef MAIN_HELPERS
#define MAIN_HELPERS
#include <chrono>
#include <filesystem>
#include <string>
#include <thread>

std::string toUtf8(const std::wstring &wide);

inline bool fileExists(const std::string &name)
{
    if (FILE *file = fopen(name.c_str(), "r"))
    {
        fclose(file);
        return true;
    }
    else
        return false;
}

bool isPathRelative(const std::string path);

std::filesystem::path getExecutablePath();
std::filesystem::path getExecutableDirPath();

/// @brief Create a processed path that is weakly canonical, and absolute.
///        A relative path is considered relative to the main program executable path.
///        This does not mutate path.
/// @param path Path to be processed.
/// @param createDirs Should directories be created if they're missing.
/// @return Processed path
std::filesystem::path prepareAndProcessPath(std::string path, bool createDirs = true);
/// @brief Create a processed path that is weakly canonical, and absolute.
///        A relative path is considered relative to the main program executable path.
///        This does not mutate path.
/// @param path Path to be processed.
/// @param createDirs Should directories be created if they're missing.
/// @return Processed path
std::filesystem::path prepareAndProcessPath(std::filesystem::path path, bool createDirs = true);

void killOtherPerpetualInstances();
void killAllPerpetualInstances();
bool isPerpetualInstanceRunning();

inline void sleepFor(unsigned int seconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

std::string readFile(std::string_view path);
void startProgram(std::string path);

#endif /* MAIN_HELPERS */

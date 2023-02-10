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

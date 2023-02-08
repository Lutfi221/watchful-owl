#ifndef MAIN_HELPERS
#define MAIN_HELPERS
#include <string>
#include <thread>
#include <chrono>

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

std::string getExecutablePath();
std::string getExecutableDirPath();

void killOtherPerpetualInstances();
void killAllPerpetualInstances();
bool isPerpetualInstanceRunning();

inline void sleepFor(unsigned int seconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

void startProgram(std::string path);

#endif /* MAIN_HELPERS */

#include <Windows.h>
#include <filesystem>

#include "config.h"
#include "helpers.h"
#include "logger.h"

using namespace std;

int main(int argc, char **argv)
{
    FreeConsole();
    killOtherPerpetualInstances();
    auto config = loadConfig();

    auto outDirPath = filesystem::weakly_canonical(
        getExecutableDirPath() /
        filesystem::path(config.outDir));
    if (!filesystem::exists(outDirPath))
        filesystem::create_directories(outDirPath);

    while (true)
    {
        captureAndAppend(config);
        sleepFor(config.loggingInterval);
    }
}

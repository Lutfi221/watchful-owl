#include <Windows.h>
#include <filesystem>

#include "config.h"
#include "helpers.h"
#include "logger.h"

using namespace std;

int WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nShowCmd)
{
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

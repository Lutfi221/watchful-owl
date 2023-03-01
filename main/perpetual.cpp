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

    auto lg = logger::Logger(&config);
    while (true)
    {
        lg.captureAndAppend();
        sleepFor(config.loggingInterval);
    }
}

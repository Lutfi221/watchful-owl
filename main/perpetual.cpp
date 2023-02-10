#include <Windows.h>

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
    while (true)
    {
        captureAndAppend(config);
        sleepFor(config.loggingInterval);
    }
}

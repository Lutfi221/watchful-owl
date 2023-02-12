#include <Windows.h>

#include "config.h"
#include "helpers.h"
#include "logger.h"

using namespace std;

int main(int argc, char **argv)
{
    FreeConsole();
    killOtherPerpetualInstances();
    auto config = loadConfig();
    while (true)
    {
        captureAndAppend(config);
        sleepFor(config.loggingInterval);
    }
}

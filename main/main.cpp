#include <iostream>
#include <filesystem>
#include "ftxui/component/screen_interactive.hpp"

#include "constants.hpp"
#include "ui/ui.h"
#include "config.h"
#include "helpers.h"

using namespace std;
namespace fs = std::filesystem;

enum LoopStatus
{
    LoopResume,
    LoopStop
};

LoopStatus loop(ftxui::ScreenInteractive *screen, Config *config)
{
    string perpetualExePath;
    bool isInstanceRunning = isPerpetualInstanceRunning();
    string mainDesc = isInstanceRunning
                          ? "Watchful Owl is ACTIVE and currently logging your activity."
                          : "Watchful Owl is currently INACTIVE.";
    vector<string> entries = {
        isInstanceRunning
            ? "Deactivate Watchful Owl"
            : "Activate Watchful Owl",
        "Enable Autorun",
        "Exit"};

    int s = promptSelection(screen, &entries, "Main Menu", mainDesc);

    switch (s)
    {
    case 0: /* activate or deactivate perpetual owl */
        if (isInstanceRunning)
        {
            killAllPerpetualInstances();
            break;
        }
        perpetualExePath = (fs::path(getExecutableDirPath()) /
                            fs::path(constants::PERPETUAL_EXE_FILENAME))
                               .u8string();
        startProgram(perpetualExePath);
        break;
    case 1:
        /* enable autorun */
        break;
    default:
        return LoopStop;
    }
    return LoopResume;
}

int main(int argc, char **argv)
{
    auto screen = ftxui::ScreenInteractive::Fullscreen();
    auto config = loadConfig();
    while (true)
    {
        if (loop(&screen, &config) == LoopStop)
            return 0;
    }
    return 0;
}

#include <ctime>
#include <iostream>
#include <fstream>
#include <Windows.h>
#include "ftxui/component/screen_interactive.hpp"

#include "ui/ui.h"
#include "config.h"
#include "capturer.h"
#include "logger.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

json generateBasicLogEntry(Config config, time_t timestamp);
void captureAndAppend(Config config);

int main(int argc, char **argv)
{
    auto screen = ftxui::ScreenInteractive::Fullscreen();
    vector<string> entries = {"Activate Watchful Owl", "Enable Autorun", "Exit"};
    int s = promptSelection(&screen, &entries, "Main Menu", "Watchful Owl is currently INACTIVE.");

    switch (s)
    {
    case 0:
        /* activate watchful owl */
        break;
    case 1:
        /* enable autorun */
        break;
    default:
        return 0;
        break;
    }
}

void captureAndAppend(Config config)
{
    time_t timestamp = time(nullptr);
    json entry;
    UINT durationSinceLastInput = getDurationSinceLastInput();
    if (durationSinceLastInput > config.idleThreshold)
    {
        entry["timestamp"] = timestamp;
        entry["durationSinceLastInput"] = durationSinceLastInput;
    }
    else
        entry = generateBasicLogEntry(config, timestamp);
    appendLogEntry(entry, config, timestamp);
}

json generateBasicLogEntry(Config config, time_t timestamp)
{
    json entry;
    entry["time"] = timestamp;

    vector<AppRecord> apps;
    getOpenedApps(&apps);

    entry["apps"] = json::array();

    for (auto const &appRecord : apps)
    {
        entry["apps"].push_back({{"title", appRecord.title},
                                 {"path", appRecord.path}});
        if (appRecord.isActive)
            entry["apps"].back()["isActive"] = true;
    };

    return entry;
}

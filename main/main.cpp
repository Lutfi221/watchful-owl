#include <ctime>
#include <iostream>
#include <fstream>
#include <Windows.h>
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
    if (argc == 1)
    {
        cout << "Use `--capture` to append a log entry to a log file." << endl
             << "Use `--perpetual` to perpetually append a log entry every n minutes"
             << " as specified in `config.json`.";
        return 0;
    }
    auto config = loadConfig(true);
    if (string(argv[1]) == "--capture")
    {
        captureAndAppend(config);
        return 0;
    }
    if (string(argv[1]) == "--perpetual")
    {
        while (true)
        {
            captureAndAppend(config);
            Sleep(config.loggingInterval * 1000);
        }
    }
    return 0;
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

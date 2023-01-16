#include <ctime>
#include <iostream>
#include <fstream>
#include "config.h"
#include "capturer.h"
#include "logger.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

json generateBasicLogEntry(Config config, time_t timestamp);

int main()
{
    auto config = loadConfig(true);
    time_t timestamp = time(nullptr);

    json entry = generateBasicLogEntry(config, timestamp);

    appendLogEntry(entry, config, timestamp);

    return 0;
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

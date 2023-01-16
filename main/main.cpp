#include <ctime>
#include <iostream>
#include <fstream>
#include "config.h"
#include "capturer.h"
#include "logger.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

int main()
{
    auto config = loadConfig(true);

    json entry;
    time_t timestamp = time(nullptr);
    entry["time"] = timestamp;

    vector<AppRecord> apps;
    getOpenedApps(&apps);

    entry["apps"] = json::array();

    for (auto const &appRecord : apps)
    {
        entry["apps"].push_back({{"title", appRecord.title},
                                 {"path", appRecord.path},
                                 {"isActive", appRecord.isActive}});
    };

    appendLogEntry(entry, config, timestamp);

    return 0;
}

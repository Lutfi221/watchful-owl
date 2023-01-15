#include <ctime>
#include <iostream>
#include "capturer.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

int main()
{
    json entry;
    entry["time"] = time(nullptr);

    vector<AppRecord> apps;
    getOpenedApps(&apps);

    entry["apps"] = json::array();

    for (auto const &appRecord : apps)
    {
        entry["apps"].push_back({
            {"title", appRecord.title},
            {"path", appRecord.path},
            {"isActive", appRecord.isActive}
        });
    };

    cout << entry.dump(4) << endl;

    return 0;
}

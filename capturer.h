#ifndef CAPTURER
#define CAPTURER

#include <string>
#include <vector>

struct AppRecord {
    std::string path;
    std::string title;
    bool isActive = false;
};

struct LogEntry {
    std::vector<AppRecord> apps;
    int activeApp;
};

void getOpenedApps(std::vector<AppRecord>* apps);

#endif /* CAPTURER */

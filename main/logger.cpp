#include <fstream>
#include <time.h>
#include "config.h"
#include "capturer.h"
#include "helpers.h"
#include "json.hpp"
#include <filesystem>

void appendLogEntry(nlohmann::json entry, Config config, time_t timestamp)
{
    namespace fs = std::filesystem;

    std::string outFilePath;
    char formattedTime[32];
    strftime(formattedTime, sizeof(formattedTime), "%Y%m%d", localtime(&timestamp));
    std::string outFileName = std::string(formattedTime) + ".json.log";

    if (isPathRelative(config.outDir))
    {
        auto a = fs::path(getExecutableDirPath()),
             b = fs::path(config.outDir),
             c = fs::path(outFileName);
        outFilePath = (a / b / c).u8string();
    }
    else
        outFilePath = (fs::path(config.outDir) / fs::path(outFileName)).u8string();

    std::ofstream outFile;
    outFile.open(outFilePath, std::ios::out | std::ios_base::app);
    if (outFile.fail())
        throw std::ios_base::failure(std::strerror(errno));
    outFile << "\n"
            << entry.dump();
}

/// @brief Capture a snapshot
/// @param config Config
/// @param timestamp UNIX timestamp
/// @return Snapshot
nlohmann::json generateBasicLogEntry(Config config, time_t timestamp)
{
    nlohmann::json entry;
    entry["time"] = timestamp;

    std::vector<AppRecord> apps;
    getOpenedApps(&apps);

    entry["apps"] = nlohmann::json::array();

    for (auto const &appRecord : apps)
    {
        entry["apps"].push_back({{"title", appRecord.title},
                                 {"path", appRecord.path}});
        if (appRecord.isActive)
            entry["apps"].back()["isActive"] = true;
    };

    return entry;
}

/// @brief Capture snapshot, and append to log file.
/// @param config Config
void captureAndAppend(Config config)
{
    time_t timestamp = time(nullptr);
    nlohmann::json entry;
    unsigned int durationSinceLastInput = getDurationSinceLastInput();
    if (durationSinceLastInput > config.idleThreshold)
    {
        entry["timestamp"] = timestamp;
        entry["durationSinceLastInput"] = durationSinceLastInput;
    }
    else
        entry = generateBasicLogEntry(config, timestamp);
    appendLogEntry(entry, config, timestamp);
}
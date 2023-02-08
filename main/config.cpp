#include <fstream>
#include "json.hpp"
#include "config.h"
#include "helpers.h"
#include <filesystem>

inline nlohmann::json generateDefaultConfig()
{
    return nlohmann::json({{"outDir", "./owl-logs"},
                           {"loggingInterval", 60},
                           {"idleThreshold", 60}});
}

Config loadConfig(bool createIfMissing)
{
    namespace fs = std::filesystem;
    auto curdir = getExecutableDirPath();
    auto configPath = (fs::path(curdir) / fs::path("config.json")).u8string();
    nlohmann::json configJ = generateDefaultConfig();

    if (fileExists(configPath))
    {
        std::ifstream f(configPath);
        nlohmann::json patch = nlohmann::json::parse(f);
        configJ.merge_patch(patch);
    }
    else
    {
        if (createIfMissing)
        {
            // Copies `default.config.json` to `config.json`
            std::ofstream dst(configPath, std::ios::binary);

            dst << configJ.dump(4);
        }
    }

    struct Config config;
    config.outDir = configJ["outDir"];
    config.loggingInterval = configJ["loggingInterval"];
    config.idleThreshold = configJ["idleThreshold"];
    return config;
}
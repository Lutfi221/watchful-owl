#include <filesystem>
#include <fstream>

#include "json.hpp"

#include "config.h"
#include "dev-logger.h"
#include "helpers.h"

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
    auto configPath = (curdir / fs::path("config.json")).u8string();
    DEBUG("Config path generated `{}`", configPath);
    nlohmann::json configJ = generateDefaultConfig();

    if (fileExists(configPath))
    {
        DEBUG("Config file exists");
        std::ifstream f(configPath);
        nlohmann::json patch = nlohmann::json::parse(f);
        configJ.merge_patch(patch);
        INFO("Read config from `{}`", configPath);
    }
    else
    {
        INFO("Config file doesn't exists, using default config");
        if (createIfMissing)
        {
            // Copies `default.config.json` to `config.json`
            std::ofstream dst(configPath, std::ios::binary);
            dst << configJ.dump(4);
            INFO("Created default config file at `{}`", configPath);
        }
    }

    struct Config config;
    config.outDir = configJ["outDir"];
    config.loggingInterval = configJ["loggingInterval"];
    config.idleThreshold = configJ["idleThreshold"];
    INFO("Loaded config `{}`", configJ.dump());
    return config;
}

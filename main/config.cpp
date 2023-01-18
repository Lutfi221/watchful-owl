#include <fstream>
#include <filesystem>
#include "json.hpp"
#include "config.h"
#include "helpers.h"
#include "cppath.hpp"

inline nlohmann::json generateDefaultConfig()
{
    return nlohmann::json({{"outDir", "./owl-logs"}});
}

Config loadConfig(bool createIfMissing)
{
    const auto curdir = cpppath::dirname(getExecutablePath());
    const auto configPath = cpppath::join({curdir, "config.json"});
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
    return config;
}
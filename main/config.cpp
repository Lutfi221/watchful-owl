#include <fstream>
#include <filesystem>
#include "json.hpp"
#include "config.h"
#include "helpers.h"

Config loadConfig(bool createIfMissing)
{
    std::ifstream f("default.config.json");
    nlohmann::json configJ = nlohmann::json::parse(f);

    if (fileExists("config.json"))
    {
        std::ifstream f("config.json");
        nlohmann::json patch = nlohmann::json::parse(f);
        configJ.merge_patch(patch);
    }
    else
    {
        if (createIfMissing)
        {
            // Copies `default.config.json` to `config.json`
            std::ifstream src("default.config.json", std::ios::binary);
            std::ofstream dst("config.json", std::ios::binary);

            dst << src.rdbuf();
        }
    }

    struct Config config;
    config.outDir = configJ["outDir"];
    return config;
}
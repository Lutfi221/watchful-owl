#include <filesystem>
#include <fstream>

#include "json.hpp"

#include "config.h"
#include "dev-logger.h"
#include "helpers.h"

// inline nlohmann::json generateDefaultConfig()
// {
//     return nlohmann::json({{"outDir", "./owl-logs"},
//                            {"loggingInterval", 60},
//                            {"idleThreshold", 60},
//                            {"encryption",
//                             {"rsaPublicKeyPath", ""},
//                             {"enabled", false}}});
// }

Config loadConfig(bool createIfMissing)
{
    namespace fs = std::filesystem;
    struct Config config;
    auto curdir = getExecutableDirPath();
    auto configPath = (curdir / fs::path("config.json")).u8string();
    DEBUG("Config path generated `{}`", configPath);

    // Fill it with the default config.
    nlohmann::json configJ = config;

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

    config = configJ.get<Config>();
    INFO("Loaded config `{}`", configJ.dump());
    return config;
}

void to_json(nlohmann::json &j, const Config &c)
{
    j = nlohmann::json{
        {"outDir", c.outDir},
        {"loggingInterval", c.loggingInterval},
        {"idleThreshold", c.idleThreshold},
    };
    j["encryption"] = nlohmann::json{
        {"enabled", c.encryption.enabled},
        {"rsaPublicKeyPath", c.encryption.rsaPublicKeyPath}};
};

void from_json(const nlohmann::json &j, Config &c)
{
    j.at("outDir").get_to(c.outDir);
    j.at("loggingInterval").get_to(c.loggingInterval);
    j.at("idleThreshold").get_to(c.idleThreshold);
    j.at("encryption").at("enabled").get_to(c.encryption.enabled);
    j.at("encryption").at("rsaPublicKeyPath").get_to(c.encryption.rsaPublicKeyPath);
};
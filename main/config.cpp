#include <filesystem>
#include <fstream>

#include "json.hpp"

#include "config.h"
#include "constants.hpp"
#include "dev-logger.h"
#include "helpers.h"

Config loadConfig(bool createIfMissing)
{
    std::string configPath = constants::CONFIG_PATH;
    Config config;
    // Initialize the json object with the default values.
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

void saveJson(nlohmann::json data, std::string path)
{
    std::ofstream dst(path, std::ios::binary);
    dst << data.dump(4);
}

void saveConfig(Config *config)
{
    nlohmann::json configJ = *config;
    saveJson(configJ, constants::CONFIG_PATH);
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
        {"rsaPublicKeyPath", c.encryption.rsaPublicKeyPath},
        {"rsaPrivateKeyPath", c.encryption.rsaPrivateKeyPath},
        {"saltPath", c.encryption.saltPath}};
};

void from_json(const nlohmann::json &j, Config &c)
{
    j.at("outDir").get_to(c.outDir);
    j.at("loggingInterval").get_to(c.loggingInterval);
    j.at("idleThreshold").get_to(c.idleThreshold);
    j.at("encryption").at("enabled").get_to(c.encryption.enabled);
    j.at("encryption").at("rsaPublicKeyPath").get_to(c.encryption.rsaPublicKeyPath);
    j.at("encryption").at("rsaPrivateKeyPath").get_to(c.encryption.rsaPrivateKeyPath);
    j.at("encryption").at("saltPath").get_to(c.encryption.saltPath);
};
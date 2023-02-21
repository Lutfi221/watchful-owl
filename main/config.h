#ifndef MAIN_CONFIG
#define MAIN_CONFIG
#include "json.hpp"
#include <string>

struct EncryptionConfig
{
    std::string rsaPublicKeyPath = "./crypto/main.rsa-public.data";
    std::string rsaPrivateKeyPath = "./crypto/main.rsa-private.data.enc";
    std::string saltPath = "./crypto/main.salt.data";
    bool enabled = false;
};

struct Config
{
    std::string outDir = "./owl-logs";
    unsigned int loggingInterval = 60;
    // How many seconds to consider the user as idle
    // and temporarily stop logging until the user is
    // active again.
    unsigned int idleThreshold = 60;
    EncryptionConfig encryption;
};

Config loadConfig(bool createIfMissing = 0);
void saveConfig(Config *config);

void to_json(nlohmann::json &j, const Config &c);
void from_json(const nlohmann::json &j, Config &c);

#endif /* MAIN_CONFIG */

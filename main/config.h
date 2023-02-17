#ifndef MAIN_CONFIG
#define MAIN_CONFIG
#include <string>

struct EncryptionConfig
{
    std::string rsaPublicKeyPath = "";
    bool enabled = false;
};

struct Config
{
    std::string outDir;
    unsigned int loggingInterval = 60;
    // How many seconds to consider the user as idle
    // and temporarily stop logging until the user is
    // active again.
    unsigned int idleThreshold = 60;
    EncryptionConfig encryption;
};

Config loadConfig(bool createIfMissing = 0);

#endif /* MAIN_CONFIG */
